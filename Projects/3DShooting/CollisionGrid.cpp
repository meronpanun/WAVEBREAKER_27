#include "CollisionGrid.h"
#include "Collision.h"
#include "EnemyBase.h"
#include "Game.h"
#include "Player.h"
#include <cmath>
#include <string>
#include <algorithm>

bool CollisionGrid::s_drawGrid = false;
bool CollisionGrid::s_useSpatialPartitioning = true;

CollisionGrid::CollisionGrid()
    : m_minArea(VGet(0, 0, 0))
    , m_maxArea(VGet(0, 0, 0))
    , m_cellSize(0)
    , m_width(0)
    , m_height(0)
    , m_totalQueries(0)
    , m_totalEntitiesChecked(0)
    , m_totalSearchTime(0)
    , m_displayedSearchTime(0)
    , m_displayTimer(0)
    , m_totalEnemies(0)
{
}

CollisionGrid::~CollisionGrid()
{
}

void CollisionGrid::Init(const VECTOR& minArea, const VECTOR& maxArea, float cellSize)
{
    m_minArea = minArea;
    m_maxArea = maxArea;
    m_cellSize = cellSize;

    m_width = (int)ceilf((maxArea.x - minArea.x) / cellSize);
    m_height = (int)ceilf((maxArea.z - minArea.z) / cellSize);

    // 境界チェックに余裕を持たせる
    m_width += 2;
    m_height += 2;

    m_cells.assign(m_width * m_height, std::vector<EnemyBase*>());
    m_stageCells.assign(m_width * m_height, std::vector<const Stage::StageCollisionData*>());
    m_accessedCells.assign(m_width * m_height, 0);
    m_cachedHeights.assign((m_width + 1) * (m_height + 1), minArea.y + 1.0f);
}

void CollisionGrid::Clear()
{
    for (auto& cell : m_cells)
    {
        cell.clear();
    }
    for (auto& cell : m_stageCells)
    {
        cell.clear();
    }
}

void CollisionGrid::ClearEnemies()
{
    for (auto& cell : m_cells)
    {
        cell.clear();
    }
}

void CollisionGrid::ResetAccessFlags()
{
    for (auto& val : m_accessedCells)
    {
        if (val > 0) val--;
    }
}

void CollisionGrid::ResetStats()
{
    m_totalQueries = 0;
    m_totalEntitiesChecked = 0;
    m_totalSearchTime = 0;
}

void CollisionGrid::CalculateHeights(const std::vector<Stage::StageCollisionData>& collisionData)
{
    if (collisionData.empty()) return;

    for (int z = 0; z <= m_height; ++z)
    {
        for (int x = 0; x <= m_width; ++x)
        {
            float posX = m_minArea.x + x * m_cellSize;
            float posZ = m_minArea.z + z * m_cellSize;

            VECTOR start = VGet(posX, 10000.0f, posZ);
            VECTOR dir = VGet(0, -1.0f, 0);
            float minDist = FLT_MAX;
            bool hit = false;

            for (const auto& col : collisionData)
            {
                float t;
                if (Collision::IntersectRayTriangle(start, dir, col.v1, col.v2, col.v3, t))
                {
                    if (t < minDist)
                    {
                        minDist = t;
                        hit = true;
                    }
                }
            }
            m_cachedHeights[z * (m_width + 1) + x] = hit ? (start.y - minDist + 1.5f) : (m_minArea.y + 1.0f);
        }
    }
}

void CollisionGrid::RegisterStageTriangle(const Stage::StageCollisionData& triangle)
{
    // 三角形のバウンディングボックスを計算
    float minX = (std::min)({ triangle.v1.x, triangle.v2.x, triangle.v3.x });
    float maxX = (std::max)({ triangle.v1.x, triangle.v2.x, triangle.v3.x });
    float minZ = (std::min)({ triangle.v1.z, triangle.v2.z, triangle.v3.z });
    float maxZ = (std::max)({ triangle.v1.z, triangle.v2.z, triangle.v3.z });

    // バウンディングボックスが重なる全セルを特定
    int startX, startZ, endX, endZ;
    GetCellIndices(VGet(minX, 0, minZ), startX, startZ);
    GetCellIndices(VGet(maxX, 0, maxZ), endX, endZ);

    // 範囲をグリッド内に収める
    startX = (std::max)(0, startX);
    startZ = (std::max)(0, startZ);
    endX = (std::min)(m_width - 1, endX);
    endZ = (std::min)(m_height - 1, endZ);

    for (int z = startZ; z <= endZ; ++z)
    {
        for (int x = startX; x <= endX; ++x)
        {
            m_stageCells[z * m_width + x].push_back(&triangle);
        }
    }
}

void CollisionGrid::RegisterEnemy(EnemyBase* enemy)
{
    if (!enemy)
        return;

    int cellIndex = GetCellIndex(enemy->GetPos());
    if (cellIndex != -1)
    {
        m_cells[cellIndex].push_back(enemy);
        m_totalEnemies++;
    }
}

void CollisionGrid::GetNeighbors(const VECTOR& pos, std::vector<EnemyBase*>& outNeighbors, bool persist) const
{
    LONGLONG startTime = GetNowHiPerformanceCount();
    m_totalQueries++;

    // 空間分割を使用しない場合（総当たりモード）
    if (!s_useSpatialPartitioning)
    {
        for (int z = 0; z < m_height; ++z)
        {
            for (int x = 0; x < m_width; ++x)
            {
                const auto& targetCell = m_cells[z * m_width + x];
                m_totalEntitiesChecked += (int)targetCell.size();
                outNeighbors.insert(outNeighbors.end(), targetCell.begin(), targetCell.end());
            }
        }
        m_totalSearchTime += GetNowHiPerformanceCount() - startTime;
        return;
    }

    int cx, cz;
    GetCellIndices(pos, cx, cz);
    
    // 指定位置の周囲9マスをチェック
    for (int z = cz - 1; z <= cz + 1; ++z)
    {
        for (int x = cx - 1; x <= cx + 1; ++x)
        {
            if (x >= 0 && x < m_width && z >= 0 && z < m_height)
            {
                int index = z * m_width + x;
                // デバッグ用アクセスフラグ。persist=trueなら維持時間を設定、falseなら即時クリア対象
                const_cast<CollisionGrid*>(this)->m_accessedCells[index] = persist ? 30 : 1;
                
                const auto& targetCell = m_cells[index];
            
                m_totalEntitiesChecked += (int)targetCell.size();
                outNeighbors.insert(outNeighbors.end(), targetCell.begin(), targetCell.end());
            }
        }
    }
    m_totalSearchTime += GetNowHiPerformanceCount() - startTime;
}

void CollisionGrid::GetNearbyTriangles(const VECTOR& pos, std::vector<const Stage::StageCollisionData*>& outTriangles) const
{
    if (!s_useSpatialPartitioning)
    {
        for (const auto& cell : m_stageCells)
        {
            for (const auto* tri : cell)
            {
                // 重複を避けるために必要だが、ここでは単純化のため全て追加し、
                // 呼び出し側で対処するか、あるいは重複を許容する（判定回数は増えるが全件よりはマシ）
                outTriangles.push_back(tri);
            }
        }
        return;
    }

    int cx, cz;
    GetCellIndices(pos, cx, cz);

    for (int z = cz - 1; z <= cz + 1; ++z)
    {
        for (int x = cx - 1; x <= cx + 1; ++x)
        {
            if (x >= 0 && x < m_width && z >= 0 && z < m_height)
            {
                int index = z * m_width + x;
                const auto& targetCell = m_stageCells[index];
                outTriangles.insert(outTriangles.end(), targetCell.begin(), targetCell.end());
            }
        }
    }

    // 同一三角形が複数セルにまたがる場合の重複を削除
    std::sort(outTriangles.begin(), outTriangles.end());
    outTriangles.erase(std::unique(outTriangles.begin(), outTriangles.end()), outTriangles.end());
}

int CollisionGrid::GetCellIndex(const VECTOR& pos) const
{
    int x, z;
    GetCellIndices(pos, x, z);
    if (x < 0 || x >= m_width || z < 0 || z >= m_height)
    {
        return -1;
    }
    return z * m_width + x;
}

void CollisionGrid::GetCellIndices(const VECTOR& pos, int& x, int& z) const
{
    float localX = pos.x - m_minArea.x;
    float localZ = pos.z - m_minArea.z;

    x = static_cast<int>(std::floor(localX / m_cellSize));
    z = static_cast<int>(std::floor(localZ / m_cellSize));
}

void CollisionGrid::Draw(const std::vector<Stage::StageCollisionData>& collisionData) const
{
    if (!s_drawGrid) return;

    // プレイヤーの周囲のみ描画して負荷を抑える
    VECTOR playerPos = VGet(0, 0, 0);
    if (Game::m_pPlayer) playerPos = Game::m_pPlayer->GetPos();
    const float drawRange = 2500.0f; 
    const float drawRangeSq = drawRange * drawRange;

    // 深度テストを有効にし、モデルに隠れるようにする
    SetUseZBuffer3D(true);
    SetWriteZBuffer3D(false);

    unsigned int lineColor = 0x646464;
    unsigned int activeCellColor = 0x00FF00;
    unsigned int searchedCellColor = 0xFFFF00;
    unsigned int textColor = 0xFFFFFF;

    auto GetCachedHeight = [&](int x, int z) {
        if (x < 0 || x > m_width || z < 0 || z > m_height) return m_minArea.y + 1.0f;
        return m_cachedHeights[z * (m_width + 1) + x];
    };

    // グリッド線の描画
    // 縦線
    for (int x = 0; x <= m_width; ++x)
    {
        float posX = m_minArea.x + x * m_cellSize;
        for (int z = 0; z < m_height; ++z)
        {
            float posZ1 = m_minArea.z + z * m_cellSize;
            float posZ2 = m_minArea.z + (z + 1) * m_cellSize;
            
            // プレイヤーからの距離チェック
            float dx = posX - playerPos.x;
            float dz = ((posZ1 + posZ2) * 0.5f) - playerPos.z;
            if (dx * dx + dz * dz > drawRangeSq) continue;

            DrawLine3D(VGet(posX, GetCachedHeight(x, z), posZ1), VGet(posX, GetCachedHeight(x, z + 1), posZ2), lineColor);
        }
    }

    // 横線
    for (int z = 0; z <= m_height; ++z)
    {
        float posZ = m_minArea.z + z * m_cellSize;
        for (int x = 0; x < m_width; ++x)
        {
            float posX1 = m_minArea.x + x * m_cellSize;
            float posX2 = m_minArea.x + (x + 1) * m_cellSize;

            // プレイヤーからの距離チェック
            float dx = ((posX1 + posX2) * 0.5f) - playerPos.x;
            float dz = posZ - playerPos.z;
            if (dx * dx + dz * dz > drawRangeSq) continue;

            DrawLine3D(VGet(posX1, GetCachedHeight(x, z), posZ), VGet(posX2, GetCachedHeight(x + 1, z), posZ), lineColor);
        }
    }

    // 敵がいるセルをハイライト
    for (int z = 0; z < m_height; ++z)
    {
        for (int x = 0; x < m_width; ++x)
        {
            float minX = m_minArea.x + x * m_cellSize;
            float minZ = m_minArea.z + z * m_cellSize;
            
            // プレイヤーからの距離チェック
            float dx = (minX + m_cellSize * 0.5f) - playerPos.x;
            float dz = (minZ + m_cellSize * 0.5f) - playerPos.z;
            if (dx * dx + dz * dz > drawRangeSq) continue;

            int index = z * m_width + x;
            bool hasEnemies = !m_cells[index].empty();
            bool isAccessed = m_accessedCells[index] > 0;

            if (hasEnemies || isAccessed)
            {
                float maxX = minX + m_cellSize;
                float maxZ = minZ + m_cellSize;

                float h00 = GetCachedHeight(x, z);
                float h10 = GetCachedHeight(x + 1, z);
                float h11 = GetCachedHeight(x + 1, z + 1);
                float h01 = GetCachedHeight(x, z + 1);

                // 敵が存在するセルは常に明るい緑の枠を表示
                if (hasEnemies)
                {
                    unsigned int brightGreen = 0x00FF00;
                    float offset = 1.0f; // 地面から少し離す
                    DrawLine3D(VGet(minX, h00 + offset, minZ), VGet(maxX, h10 + offset, minZ), brightGreen);
                    DrawLine3D(VGet(maxX, h10 + offset, minZ), VGet(maxX, h11 + offset, maxZ), brightGreen);
                    DrawLine3D(VGet(maxX, h11 + offset, maxZ), VGet(minX, h01 + offset, maxZ), brightGreen);
                    DrawLine3D(VGet(minX, h01 + offset, maxZ), VGet(minX, h00 + offset, minZ), brightGreen);

                    // 太く見せるために少し内側にも描画
                    float inner = 0.5f;
                    DrawLine3D(VGet(minX + inner, h00 + offset, minZ + inner), VGet(maxX - inner, h10 + offset, minZ + inner), brightGreen);
                    DrawLine3D(VGet(maxX - inner, h10 + offset, minZ + inner), VGet(maxX - inner, h11 + offset, maxZ - inner), brightGreen);
                    DrawLine3D(VGet(maxX - inner, h11 + offset, maxZ - inner), VGet(minX + inner, h01 + offset, maxZ - inner), brightGreen);
                    DrawLine3D(VGet(minX + inner, h01 + offset, maxZ - inner), VGet(minX + inner, h00 + offset, minZ + inner), brightGreen);
                }

                // 検索（アクセス）されたセルは黄色い太枠（さらに上の高さ）で表示
                if (isAccessed)
                {
                    float offset = 10.0f; // 緑より高くする
                    DrawLine3D(VGet(minX, h00 + offset, minZ), VGet(maxX, h10 + offset, minZ), searchedCellColor);
                    DrawLine3D(VGet(maxX, h10 + offset, minZ), VGet(maxX, h11 + offset, maxZ), searchedCellColor);
                    DrawLine3D(VGet(maxX, h11 + offset, maxZ), VGet(minX, h01 + offset, maxZ), searchedCellColor);
                    DrawLine3D(VGet(minX, h01 + offset, maxZ), VGet(minX, h00 + offset, minZ), searchedCellColor);
                    
                    // 少しずらして描画して太線に見せる
                    float shift = 1.0f;
                    DrawLine3D(VGet(minX+shift, h00 + offset, minZ+shift), VGet(maxX-shift, h10 + offset, minZ+shift), searchedCellColor);
                    DrawLine3D(VGet(maxX-shift, h10 + offset, minZ+shift), VGet(maxX-shift, h11 + offset, maxZ-shift), searchedCellColor);
                    DrawLine3D(VGet(maxX-shift, h11 + offset, maxZ-shift), VGet(minX+shift, h01 + offset, maxZ-shift), searchedCellColor);
                    DrawLine3D(VGet(minX+shift, h01 + offset, maxZ-shift), VGet(minX+shift, h00 + offset, minZ+shift), searchedCellColor);
                }

                if (hasEnemies)
                {
                    float avgH = (h00 + h10 + h11 + h01) * 0.25f;
                    VECTOR center = VGet((minX + maxX) * 0.5f, avgH + 5.0f, (minZ + maxZ) * 0.5f);
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%d", (int)m_cells[index].size());

                    // カメラの前方にある場合のみ描画
                    VECTOR camPos = GetCameraPosition();
                    VECTOR camTarget = GetCameraTarget();
                    VECTOR camDir = VSub(camTarget, camPos);
                    VECTOR toCenter = VSub(center, camPos);
                    
                    if (VDot(camDir, toCenter) > 0.0f)
                    {
                        VECTOR screenPos = ConvWorldPosToScreenPos(center);
                        DrawString(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), buf, textColor);
                    }
                }
            }
        }
    }
}

void CollisionGrid::DrawUI() const
{
    if (!s_drawGrid) return;

    // 2DUI描画の際は深度テストを無効化
    SetUseZBuffer3D(false);
    SetWriteZBuffer3D(false);

    // デバッグ用の凡例（説明）を描画
    int screenW = 1280;
    int screenH = 720;
    GetWindowSize(&screenW, &screenH);

    int margin = 20;
    int rectW = 240;
    int rectH = 90; 
    int x = screenW - rectW - margin;
    int y = screenH - rectH - margin - 100;

    // 半透明の背景
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(x, y, x + rectW, y + rectH, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    DrawBox(x, y, x + rectW, y + rectH, 0xffffff, false);

    // 凡例テキスト
    int textX = x + 10;
    int textY = y + 10;
    DrawBox(textX, textY + 2, textX + 12, textY + 14, 0x00FF00, true);
    DrawString(textX + 20, textY, "：敵が存在するセル", 0xffffff);

    textY += 25;
    DrawBox(textX, textY + 2, textX + 12, textY + 14, 0xFFFF00, true);
    DrawString(textX + 20, textY, "：検索・アクセス範囲", 0xffffff);

    textY += 25;
    DrawString(textX, textY, "数字：セル内の敵の数", 0xffffff);

    // パフォーマンス統計
    int statW = 320;
    int statH = 150; 
    int sx = margin;
    int sy = screenH - statH - margin - 100;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(sx, sy, sx + statW, sy + statH, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    DrawBox(sx, sy, sx + statW, sy + statH, 0xffffff, false);

    int sTextX = sx + 10;
    int sTextY = sy + 10;
    DrawString(sTextX, sTextY, "【空間分割パフォーマンス評価】", 0x00ffff);
    
    sTextY += 25;
    DrawFormatString(sTextX, sTextY, 0xffffff, "現在のモード: %s", s_useSpatialPartitioning ? "空間分割 (ON)" : "総当たり (OFF)");
    
    sTextY += 20;
    DrawFormatString(sTextX, sTextY, 0xffffff, "総敵数: %d / クエリ数: %d", m_totalEnemies, m_totalQueries);
    
    sTextY += 20;
    int fullScanChecks = m_totalEnemies * m_totalQueries;
    float reduction = 0.0f;
    if (fullScanChecks > 0)
    {
        reduction = (1.0f - (float)m_totalEntitiesChecked / fullScanChecks) * 100.0f;
    }
    
    unsigned int statColor = s_useSpatialPartitioning ? 0x00ff00 : 0xffaa00;
    DrawFormatString(sTextX, sTextY, statColor, "判定対象削減率: %.1f%%", s_useSpatialPartitioning ? reduction : 0.0f);
    
    sTextY += 20;
    DrawFormatString(sTextX, sTextY, 0xffffff, "実判定数: %d (総当りなら: %d)", m_totalEntitiesChecked, fullScanChecks);

    sTextY += 25;
    
    // 表示更新頻度を落とす
    m_displayTimer--;
    if (m_displayTimer <= 0)
    {
        m_displayedSearchTime = m_totalSearchTime;
        m_displayTimer = 30;
    }
    DrawFormatString(sTextX, sTextY, 0xffff00, "検索処理時間: %lld us (マイクロ秒)", m_displayedSearchTime);
}
