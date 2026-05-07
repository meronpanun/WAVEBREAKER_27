#pragma once
#include "DxLib.h"
#include "Stage.h"
#include <vector>

class EnemyBase;

class CollisionGrid
{
public:
    CollisionGrid();
    ~CollisionGrid();

    /// <summary>
    /// グリッドの初期化
    /// </summary>
    /// <param name="minArea">領域の最小座標</param>
    /// <param name="maxArea">領域の最大座標</param>
    /// <param name="cellSize">セルのサイズ</param>
    void Init(const VECTOR& minArea, const VECTOR& maxArea, float cellSize);

    /// <summary>
    /// グリッドのクリア
    /// </summary>
    void Clear();
    void ClearEnemies(); // 敵のみをクリア
    void ResetAccessFlags();
    void ResetStats();
    void CalculateHeights(const std::vector<Stage::StageCollisionData>& collisionData);

    /// <summary>
    /// ステージの三角形ポリゴンを登録
    /// </summary>
    void RegisterStageTriangle(const Stage::StageCollisionData& triangle);

    /// <summary>
    /// 敵をグリッドに登録
    /// </summary>
    /// <param name="enemy">登録する敵</param>
    void RegisterEnemy(EnemyBase* enemy);

    /// <summary>
    /// 指定位置周辺の敵を取得
    /// </summary>
    /// <param name="pos">検索中心座標</param>
    /// <param name="outNeighbors">結果を格納するベクタ</param>
    void GetNeighbors(const VECTOR& pos,
        std::vector<EnemyBase*>& outNeighbors, bool persist = true) const;

    /// <summary>
    /// 指定位置周辺のステージポリゴンを取得
    /// </summary>
    void GetNearbyTriangles(const VECTOR& pos,
        std::vector<const Stage::StageCollisionData*>& outTriangles) const;

    /// <summary>
    /// グリッドのデバッグ描画
    /// </summary>
    void Draw(const std::vector<Stage::StageCollisionData>& collisionData = {}) const;
    void DrawUI() const;

    static void SetDrawGrid(bool draw) { s_drawGrid = draw; }
    static bool IsDrawGrid() { return s_drawGrid; }

    static void SetUseSpatialPartitioning(bool use) { s_useSpatialPartitioning = use; }
    static bool IsUseSpatialPartitioning() { return s_useSpatialPartitioning; }

    // パフォーマンス計測用
    int GetTotalQueries() const { return m_totalQueries; }
    int GetTotalEntitiesChecked() const { return m_totalEntitiesChecked; }
    int GetTotalEnemies() const { return m_totalEnemies; }
    void SetTotalEnemies(int count) { m_totalEnemies = count; }
    LONGLONG GetTotalSearchTime() const { return m_totalSearchTime; }

private:
    int GetCellIndex(const VECTOR& pos) const;
    void GetCellIndices(const VECTOR& pos, int& x, int& z) const;

private:
    std::vector<std::vector<EnemyBase*>> m_cells;
    std::vector<std::vector<const Stage::StageCollisionData*>> m_stageCells; // ステージポリゴン用
    mutable std::vector<int> m_accessedCells; // デバッグ用：アクセスされたセルを記録
    std::vector<float> m_cachedHeights;       // デバッグ用：地形の高さをキャッシュ
    VECTOR m_minArea;
    VECTOR m_maxArea;
    float m_cellSize;
    int m_width;
    int m_height;

    // 計測用
    mutable int m_totalQueries;
    mutable int m_totalEntitiesChecked;
    mutable LONGLONG m_totalSearchTime;
    mutable LONGLONG m_displayedSearchTime; // 表示用にホールドする時間
    mutable int m_displayTimer;             // 表示更新用タイマー
    int m_totalEnemies;

    static bool s_drawGrid;
    static bool s_useSpatialPartitioning;
};
