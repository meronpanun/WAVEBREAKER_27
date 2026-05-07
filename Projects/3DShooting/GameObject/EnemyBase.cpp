#include "EnemyBase.h"
#include "Bullet.h"
#include "TransformDataLoader.h"
#include "CapsuleCollider.h"
#include "Collider.h"
#include "Collision.h"
#include "Effect.h"
#include "EffekseerForDXLib.h"
#include "Game.h"
#include "CollisionGrid.h"
#include <algorithm>
#include "Stage.h"
#include "TaskTutorialManager.h"

namespace EnemyConstants
{
    constexpr int kDefaultHitDisplayDuration = 60; // 1秒間表示
    constexpr float kDefaultInitialHP        = 100.0f;    // デフォルトの初期体力
    constexpr float kDefaultCooldownMax      = 60;      // 攻撃クールダウンの最大値
    constexpr float kDefaultAttackPower = 10.0f;   // 攻撃力

	// 物理関連定数
    constexpr float kCapsuleHeight   = 100.0f;
    constexpr float kCapsuleRadius   = 30.0f;
    constexpr float kColliderYOffset = 60.0f;
    constexpr float kGravity         = 0.35f;

	// 描画最適化関連定数
    constexpr float kThrottlingSuperLongRangeSq = 1500.0f * 1500.0f;
    constexpr float kThrottlingLongRangeSq      = 800.0f * 800.0f;
    constexpr float kThrottlingMidRangeSq       = 400.0f * 400.0f;
    constexpr float kThrottlingViewCheckDistSq  = 300.0f * 300.0f;
    constexpr float kThrottlingFOVThreshold     = 0.4f;
    constexpr int kUpdateIntervalSuperLong      = 10;
    constexpr int kUpdateIntervalLong           = 5; // 3から5に増加
    constexpr int kUpdateIntervalMid            = 2;
    constexpr int kUpdateIntervalDefault        = 1;
    constexpr float kBulletBroadPhaseDistSq     = 300.0f * 300.0f;

	// 放物線運動関連定数
    constexpr float kParabolicMinTime       = 20.0f;
    constexpr float kParabolicTimeFactor    = 2.0f;
    constexpr float kParabolicGravityFactor = 0.5f;

	// デバッグ表示関連定数
    constexpr int kDebugBoxPaddingX = 20;
    constexpr int kDebugBoxPaddingY = 10;
    constexpr float kDebugBoxValidYRatio = 0.15f;
}

int EnemyBase::s_drawCount = 0;
int EnemyBase::s_aiUpdateCount = 0;
int EnemyBase::s_totalCount = 0;
bool EnemyBase::s_shouldShowDamage = false;
float EnemyBase::s_debugLastDamage = 0.0f;
int EnemyBase::s_debugDamageTimer = 0;
std::string EnemyBase::s_debugHitInfo = "";

EnemyBase::EnemyBase() 
    : m_pos{ 0, 0, 0 }
    , m_modelHandle(-1)
    , m_pTargetPlayer(nullptr)
    , m_hp(EnemyConstants::kDefaultInitialHP)
    , m_maxHp(EnemyConstants::kDefaultInitialHP)
    , m_lastHitPart(HitPart::None)
    , m_lastTackleId(-1)
    , m_hitDisplayTimer(0)
    , m_isAlive(true)
    , m_hasTakenTackleDamage(false)
    , m_attackCooldown(0)
    , m_attackCooldownMax(static_cast<int>(EnemyConstants::kDefaultCooldownMax))
    , m_attackPower(EnemyConstants::kDefaultAttackPower)
    , m_attackHitFrame(0)
    , m_isAttacking(false)
    , m_isActive(true)
    , m_verticalVelocity(0.0f)
    , m_isGrounded(false)
    , m_updateFrameCount(0)
    , m_aiUpdateInterval(1)
    , m_isSimpleMode(false)
    , m_shouldUpdateAI(true)
    , m_chaseSpeed(0.0f)
{
}

void EnemyBase::CheckHitAndDamage(std::vector<Bullet>& bullets, Effect* pEffect)
{
    HitPart determinedHitPart = HitPart::None;
    float minHitDistSq = FLT_MAX;

    // 最も近いヒットした弾を探す
    int hitBulletIndex = FindClosestHitBullet(bullets, determinedHitPart, minHitDistSq);

    // ヒットした場合のダメージ適用処理
    if (hitBulletIndex != -1)
    {
        ApplyBulletDamage(bullets[hitBulletIndex], determinedHitPart, minHitDistSq, pEffect);
    }
}

int EnemyBase::FindClosestHitBullet(const std::vector<Bullet>& bullets, HitPart& outPart, float& outDistSq) const
{
    int hitBulletIndex = -1;
    outDistSq = FLT_MAX;
    outPart = HitPart::None;

    for (int i = 0; i < bullets.size(); ++i)
    {
        const auto& bullet = bullets[i];
        if (!bullet.IsActive()) continue;

        // 弾のRay情報を取得
        VECTOR rayStart = bullet.GetPrevPos();
        VECTOR rayEnd = bullet.GetPos();

        // ブロードフェーズ: 敵の中心から弾丸が一定距離以上離れている場合は詳細判定をスキップ
        // 敵の高さが約100なので、中心点として少し上の座標で距離チェックを行う
        VECTOR enemyCenter = VAdd(m_pos, VGet(0, EnemyConstants::kCapsuleHeight * 0.5f, 0));
        float d1 = VSquareSize(VSub(rayStart, enemyCenter));
        float d2 = VSquareSize(VSub(rayEnd, enemyCenter));
        
        if (d1 > EnemyConstants::kBulletBroadPhaseDistSq && d2 > EnemyConstants::kBulletBroadPhaseDistSq)
        {
            continue;
        }

        // どこに当たったのかをチェック
        VECTOR currentHitPos;
        float currentHitDistSq;
        HitPart part = CheckHitPart(rayStart, rayEnd, currentHitPos, currentHitDistSq);

        if (part != HitPart::None)
        {
            if (currentHitDistSq < outDistSq)
            {
                outDistSq = currentHitDistSq;
                hitBulletIndex = i;
                outPart = part; // 最も近いヒットの部位を保持
            }
        }
    }

    return hitBulletIndex;
}

void EnemyBase::ApplyBulletDamage(Bullet& bullet, HitPart part, float distSq, Effect* pEffect)
{
    float damage = CalcDamage(bullet.GetDamage(), part);
    TakeDamage(damage, bullet.GetAttackType());

    // デバッグ表示用更新
    if (s_shouldShowDamage)
    {
        s_debugLastDamage = damage;
        s_debugDamageTimer = EnemyConstants::kDebugDamageDisplayTimer;
        if (part == HitPart::Head)
        {
            s_debugHitInfo = "(Head)";
        }
        else if (part == HitPart::Body)
        {
            s_debugHitInfo = "(Body)";
        }
        else
        {
            s_debugHitInfo = "(None)";
        }
    }

    m_lastHitPart = part;
    m_hitDisplayTimer = EnemyConstants::kDefaultHitDisplayDuration;

    // 弾が当たった位置で出血エフェクトを生成
    if (pEffect)
    {
        VECTOR hitPos = bullet.GetPos(); // 弾の現在位置を衝突位置として使用
        int handle = pEffect->PlayLossOfBlood(hitPos.x, hitPos.y, hitPos.z, 0.0f, 0.0f, 0.0f);
        if (handle != -1)
        {
            AttachedEffect effect;
            effect.handle = handle;
            effect.localOffset = VSub(hitPos, m_pos);
            m_attachedEffects.push_back(effect);
        }
    }

    bullet.Deactivate(); // 敵に当たった弾は非アクティブにする

    // ヒット時コールバック（ヒットマーク用）
    if (m_onHitCallback)
    {
        // 距離を計算して渡す
        float hitDist = sqrtf(distSq);
        m_onHitCallback(part, hitDist);
    }
}

// 敵がダメージを受ける処理
void EnemyBase::TakeDamage(float damage, AttackType type)
{
    m_lastAttackType = type;
    m_hp -= damage;
    if (m_hp <= 0.0f)
    {
        m_hp = 0.0f;
        if (m_isAlive)
        {
            m_isAlive = false;
            TaskTutorialManager::GetInstance()->NotifyEnemyKilled(m_lastAttackType);
            if (m_onDeathWithTypeCallback)
            {
                m_onDeathWithTypeCallback(m_pos, m_lastAttackType);
            }
            OnDeath(); // 敵が死亡した際にOnDeathを呼び出す
        }
    }

    // デバッグ表示用
    if (s_shouldShowDamage)
    {
        s_debugLastDamage = damage;
        s_debugDamageTimer = EnemyConstants::kDebugDamageDisplayTimer;
        switch (type)
        {
        case AttackType::Shoot:
            s_debugHitInfo = "(Shot)"; 
            break;
        case AttackType::Tackle:
            s_debugHitInfo = "(Tackle)";
            break;
        case AttackType::ShieldThrow:
            s_debugHitInfo = "(Shield)";
            break;
        case AttackType::Parry:
            s_debugHitInfo = "(Parry)";
            break;
        default:
            s_debugHitInfo = "(Unknown)";
            break;
        }
    }
}

// 敵がタックルダメージを受ける処理
void EnemyBase::TakeTackleDamage(float damage)
{
    TakeDamage(damage, AttackType::Tackle);
    m_lastHitPart = HitPart::Body;
    m_hitDisplayTimer = EnemyConstants::kDefaultHitDisplayDuration;

    // デバッグ表示用
    if (s_shouldShowDamage)
    {
        s_debugLastDamage = damage;
        s_debugDamageTimer = EnemyConstants::kDebugDamageDisplayTimer;
        s_debugHitInfo = "(Tackle)";
    }
}

void EnemyBase::UpdateStageCollision(const std::vector<Stage::StageCollisionData>& collisionData, const CollisionGrid* pGrid)
{
    // 地形判定の間引き（最適化）
    // 遠方の敵は毎フレーム地形判定を行う必要性が低いため、頻度を落とす
    if (!m_isGrounded || m_verticalVelocity != 0.0f || m_shouldUpdateAI)
    {
        // 重力適用
        m_verticalVelocity -= EnemyConstants::kGravity;
        m_pos.y += m_verticalVelocity;

        CollisionResult result = Collision::CheckStageCollision(m_pos, EnemyConstants::kCapsuleHeight, EnemyConstants::kCapsuleRadius, EnemyConstants::kColliderYOffset, collisionData, pGrid);
        m_isGrounded = result.isGrounded;

        // Y=0 平面（地面）との判定
        if (m_pos.y <= 0.0f)
        {
            m_pos.y = 0.0f;
            m_isGrounded = true;
        }

        if (m_isGrounded && m_verticalVelocity < 0.0f)
        {
            m_verticalVelocity = 0.0f;
        }
    }

    // エフェクトの追従更新
    UpdateAttachedEffects();
}

void EnemyBase::UpdateAttachedEffects()
{
    for (auto it = m_attachedEffects.begin(); it != m_attachedEffects.end(); )
    {
        if (IsEffekseer3DEffectPlaying(it->handle))
        {
            VECTOR newPos = VAdd(m_pos, it->localOffset);
            SetPosPlayingEffekseer3DEffect(it->handle, newPos.x, newPos.y, newPos.z);
            ++it;
        }
        else
        {
            it = m_attachedEffects.erase(it);
        }
    }
}


bool EnemyBase::IsTargetVisible(const VECTOR& startPos, const VECTOR& targetPos, const std::vector<Stage::StageCollisionData>& stageCollision, const CollisionGrid* pGrid)
{
    VECTOR dir = VSub(targetPos, startPos);
    float dist = VSize(dir);
    if (dist < 0.001f) return true;
    dir = VNorm(dir);

    float t;
    if (pGrid)
    {
        // 始点と終点の周囲のセルからポリゴンを取得
        std::vector<const Stage::StageCollisionData*> triangles;
        pGrid->GetNearbyTriangles(startPos, triangles);
        
        std::vector<const Stage::StageCollisionData*> endTriangles;
        pGrid->GetNearbyTriangles(targetPos, endTriangles);
        triangles.insert(triangles.end(), endTriangles.begin(), endTriangles.end());
        
        std::sort(triangles.begin(), triangles.end());
        triangles.erase(std::unique(triangles.begin(), triangles.end()), triangles.end());

        for (const auto* pCol : triangles)
        {
            if (Collision::IntersectRayTriangle(startPos, dir, pCol->v1, pCol->v2, pCol->v3, t))
            {
                if (t < dist) return false;
            }
        }
        return true;
    }

    // レイキャスト判定
    for (const auto& col : stageCollision)
    {
        if (Collision::IntersectRayTriangle(startPos, dir, col.v1, col.v2, col.v3, t))
        {
            if (t < dist) return false; // 遮蔽物あり
        }
    }
    return true;
}

VECTOR EnemyBase::CalculateParabolicVelocity(const VECTOR& startPos, const VECTOR& targetPos, float gravity, float speed)
{
    VECTOR toTarget = VSub(targetPos, startPos);

    // 滞空時間を設定 (距離に応じて調整)
    float dist = VSize(toTarget);
    // 直線より速い速度を基準にして放物線を低くする
    float time = dist / (speed * EnemyConstants::kParabolicTimeFactor);
    if (time < EnemyConstants::kParabolicMinTime) time = EnemyConstants::kParabolicMinTime; // 最低保証

    // 初速度計算
    VECTOR gravityVec = VGet(0.0f, -gravity, 0.0f);
    VECTOR term1 = VScale(toTarget, 1.0f / time);
    VECTOR term2 = VScale(gravityVec, EnemyConstants::kParabolicGravityFactor * time);

    return VSub(term1, term2);
}

// AI更新頻度の動的制御によるCPU負荷の最適化
void EnemyBase::UpdateThrottling(const VECTOR& playerPos)
{
    // 総数をカウント
    IncrementTotalCount();

    // 自身からプレイヤーまでの距離を算出
    VECTOR toPlayer = VSub(playerPos, m_pos);
    float distSq = VSquareSize(toPlayer);

    // デフォルトの設定（毎フレーム実行）
    m_aiUpdateInterval = EnemyConstants::kUpdateIntervalDefault;
    m_isSimpleMode = false;

    // 距離ベースの更新間引き処理（遠方のアクティブな敵ほどAIの計算サイクルを落とす）
    if (distSq > EnemyConstants::kThrottlingSuperLongRangeSq)
    {
        m_aiUpdateInterval = EnemyConstants::kUpdateIntervalSuperLong; // 超遠距離
    }
    else if (distSq > EnemyConstants::kThrottlingLongRangeSq)
    {
        m_aiUpdateInterval = EnemyConstants::kUpdateIntervalLong;  // 遠距離
    }
    else if (distSq > EnemyConstants::kThrottlingMidRangeSq)
    {
        m_aiUpdateInterval = EnemyConstants::kUpdateIntervalMid;   // 中距離
    }

    // 視界ベースの簡易モード移行処理（画面外におけるAI行動の簡略化）
    // ボスキャラクターは常にフル演算を行うため除外
    if (!IsBoss())
    {
        VECTOR camPos = GetCameraPosition();
        float enemyDistSq = VSquareSize(VSub(m_pos, camPos));

        // プレイヤーおよびカメラからそれぞれ一定距離以上離れている場合のみ視界限界の判定を実行
        if (distSq > EnemyConstants::kThrottlingMidRangeSq && enemyDistSq > EnemyConstants::kThrottlingViewCheckDistSq)
        {
            VECTOR camDir = VNorm(VSub(GetCameraTarget(), camPos));
            VECTOR dirToEnemy = VNorm(VSub(m_pos, camPos));
            
            // カメラ方向ベクトルとの内積を取り、完全な視界外（背後等）と判定されたら簡易モードへ移行
            if (VDot(camDir, dirToEnemy) < EnemyConstants::kThrottlingFOVThreshold)
            {
                m_isSimpleMode = true;
            }
        }
    }

    // 算出したインターバルを用いて、今回のフレームで優先してAIを更新すべきかを設定
    m_updateFrameCount++;
    m_shouldUpdateAI = (m_updateFrameCount % m_aiUpdateInterval == 0);

    // デバッグ用: AI更新が行われる場合はカウント
    if (m_shouldUpdateAI)
    {
        IncrementAIUpdateCount();
    }
}

// デバッグ用ダメージ描画
void EnemyBase::DrawDebugDamage()
{
    if (s_shouldShowDamage && s_debugDamageTimer > 0)
    {
        s_debugDamageTimer--;

        int screenW = Game::GetScreenWidth();
        int screenH = Game::GetScreenHeight();

        // 表示テキストの整形
        char text[256];
        sprintf_s(text, "Last Damage: %.1f %s", s_debugLastDamage, s_debugHitInfo.c_str());

        // テキスト幅の近似計算 (等幅フォントを前提とする)
        // DxLibのデフォルトフォント前提
        int strLen = static_cast<int>(strlen(text));
        
        // 全角半角混じりは GetDrawStringWidth が確実
        int strWidth = GetDrawStringWidth(text, strLen);

        int fontSize = GetFontSize();

        // 背景ボックスのサイズと位置
        int boxW = strWidth + EnemyConstants::kDebugBoxPaddingX * 2;
        int boxH = fontSize + EnemyConstants::kDebugBoxPaddingY * 2;

        int boxX = static_cast<int>((screenW - boxW) * 0.5f);     // 横中央
        int boxY = static_cast<int>(screenH * EnemyConstants::kDebugBoxValidYRatio); // 画面上部から15%の位置

        // 半透明背景描画 (黒, alpha=128)
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
        DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, 0x000000, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // テキスト描画 (赤)
        DrawString(boxX + EnemyConstants::kDebugBoxPaddingX, boxY + EnemyConstants::kDebugBoxPaddingY, text, 0xFF0000);
    }
}

// Transformデータをロードする
bool EnemyBase::LoadTransformData(const std::string& enemyName)
{
    auto dataList = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
    for (const auto& data : dataList)
    {
        if (data.name == enemyName)
        {
            MV1SetRotationXYZ(m_modelHandle, data.rot);
            MV1SetScale(m_modelHandle, data.scale);
            m_attackPower = data.attack;
            m_hp = data.hp;
            m_maxHp = data.hp;
            m_chaseSpeed = data.chaseSpeed;
            return true;
        }
    }
    return false;
}

// ターゲットに向かって回転する
void EnemyBase::RotateTowards(const VECTOR& targetPos, float rotationSpeed)
{
    VECTOR toTarget = VSub(targetPos, m_pos);
    toTarget.y = 0.0f;
    if (VSquareSize(toTarget) < 0.0001f) return;

    float yaw = atan2f(toTarget.x, toTarget.z) + DX_PI_F;
    float currentYaw = MV1GetRotationXYZ(m_modelHandle).y;

    float diffYaw = yaw - currentYaw;
    while (diffYaw <= -DX_PI_F) diffYaw += DX_TWO_PI_F;
    while (diffYaw > DX_PI_F) diffYaw -= DX_TWO_PI_F;

    if (fabs(diffYaw) > rotationSpeed)
    {
        currentYaw += (diffYaw > 0 ? rotationSpeed : -rotationSpeed);
    }
    else
    {
        currentYaw = yaw;
    }
    MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, currentYaw, 0.0f));
}

// 描画すべきかどうかを判定
bool EnemyBase::ShouldDraw(float drawDistSq, float nearDistSq, float dotThreshold) const
{
    // 描画カリングによる描画負荷の最適化
    VECTOR camPos = GetCameraPosition();
    VECTOR camTarget = GetCameraTarget();
    
    // カメラから敵へのベクトルと、その距離の「2乗」を算出
    // ※高コストな平方根（sqrt）計算を避けるため、距離は常に2乗（Sq）のままで比較を行う
    VECTOR toEnemy = VSub(m_pos, camPos);
    float distSq = VSquareSize(toEnemy);

    // 遠距離カリング: 描画限界距離より遠ければ即座に描画をスキップ
    if (distSq > drawDistSq) return false;
    
    // 近距離の無条件描画: 一定距離以内なら無条件で描画（画面端での不自然な見切れを防止）
    if (distSq <= nearDistSq) return true;

    // 視界カリング: カメラの正面ベクトルと敵へのベクトルの内積を利用した視野外判定
    VECTOR camDir = VNorm(VSub(camTarget, camPos));
    VECTOR dirToEnemy = VNorm(toEnemy);
    float dot = VDot(camDir, dirToEnemy);
    
    // 算出した内積パラメータが閾値（FOV）以上なら視界内として描画、それ以外ならスキップ
    return dot >= dotThreshold;
}

// プレイヤーとの衝突（押し出し）処理
void EnemyBase::ResolvePlayerCollision(const std::shared_ptr<CapsuleCollider>& playerCol, float radiusSum, float pushBackEpsilon)
{
    auto myCol = GetBodyCollider();
    if (!myCol || !playerCol) return;

    if (myCol->IsIntersects(playerCol.get()))
    {
        VECTOR enemyCenter = VScale(VAdd(myCol->GetSegmentA(), myCol->GetSegmentB()), 0.5f);
        VECTOR playerCenter = VScale(VAdd(playerCol->GetSegmentA(), playerCol->GetSegmentB()), 0.5f);
        VECTOR diff = VSub(enemyCenter, playerCenter);
        float distSq = VDot(diff, diff);
        float minDist = radiusSum;

        if (distSq < minDist * minDist && distSq > pushBackEpsilon)
        {
            float dist = std::sqrt(distSq);
            float pushBack = minDist - dist;
            if (dist > 0)
            {
                VECTOR pushDir = VSub(enemyCenter, playerCenter);
                pushDir.y = 0.0f;
                // 水平成分がある場合のみ
                if (VDot(pushDir, pushDir) > pushBackEpsilon)
                {
                    pushDir = VNorm(pushDir);
                    m_pos = VAdd(m_pos, VScale(pushDir, pushBack * 0.5f));
                }
            }
        }
    }
}

// 敵同士の衝突（押し出し）処理
void EnemyBase::ResolveEnemyCollision(const std::vector<EnemyBase*>& targets, float radius, float pushBackEpsilon)
{
    // 密集対策: 計算負荷を一定に保つため、判定対象を最大8体までに制限する
    int count = 0;
    constexpr int kMaxCollisionChecks = 8;

    for (EnemyBase* other : targets)
    {
        if (!other || other == this) continue;
        if (++count > kMaxCollisionChecks) break;

        VECTOR otherPos = other->GetPos();
        VECTOR diff = VSub(m_pos, otherPos);
        diff.y = 0.0f;
        float distSq = VDot(diff, diff);
        float minDist = radius * 2.0f;

        if (distSq < minDist * minDist && distSq > pushBackEpsilon)
        {
            float dist = std::sqrt(distSq);
            float pushBack = minDist - dist;
            if (dist > 0)
            {
                VECTOR pushDir = VNorm(diff);
                m_pos = VAdd(m_pos, VScale(pushDir, pushBack * 0.5f));
            }
        }
    }
}

// 弾のダメージを計算する（デフォルト実装）
float EnemyBase::CalcDamage(float bulletDamage, HitPart part) const
{
    if (part == HitPart::Head) return bulletDamage * 2.0f;
    if (part == HitPart::Body) return bulletDamage;
    return 0.0f;
}