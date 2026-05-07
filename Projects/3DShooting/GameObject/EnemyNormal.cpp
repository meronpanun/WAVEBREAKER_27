#include "EnemyNormal.h"
#include "Bullet.h"
#include "CapsuleCollider.h"
#include "CollisionGrid.h"
#include "DebugUtil.h"
#include "DxLib.h"
#include "EffekseerForDXLib.h"
#include "Game.h"
#include "Player.h"
#include "SceneMain.h"
#include "ScoreManager.h"
#include "SphereCollider.h"
#include "TransformDataLoader.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <string>
#include <vector>
#include "SoundManager.h"
#include "EnemyState.h"
#include "EnemyNormalState.h"

namespace EnemyNormalConstants
{
    // アニメーション関連
    constexpr char kAttackAnimName[] = "ATK"; // 攻撃アニメーション
    constexpr char kWalkAnimName[] = "WALK";  // 歩行アニメーション
    constexpr char kDeadAnimName[] = "DEAD";  // 死亡アニメーション

    const VECTOR kHeadShotPositionOffset = { 0.0f, 0.0f, 0.0f }; // オフセット

    // カプセルコライダーのサイズを定義
    constexpr float kBodyColliderRadius = 20.0f;  // 体のコライダー半径
    constexpr float kBodyColliderHeight = 110.0f; // 体のコライダー高さ
    constexpr float kHeadRadius = 12.0f;          // 頭のコライダー半径

    // 攻撃関連
    constexpr int kAttackCooldownMax = 45;       // 攻撃クールダウン時間
    constexpr float kAttackHitRadius = 45.0f;    // 攻撃の当たり判定半径
    constexpr float kAttackRangeRadius = 120.0f; // 攻撃範囲の半径

    // 追跡関連
    constexpr float kChaseStopDistance = 50.0f; // 追跡停止距離

    // 攻撃後の硬直時間
    constexpr int kAttackEndDelay = 20;

    // ダメージ（怯み）時間
    constexpr int kDamageDuration = 30;

    // 描画距離
    constexpr float kDrawDistanceSq = 5000.0f * 5000.0f; // 16000から5000に縮小
    constexpr float kDrawNearDistanceSq = 300.0f * 300.0f;
    constexpr float kDrawDotThreshold = 0.4f;

    // 押し出し
    constexpr float kPushBackEpsilon = 0.0001f;
}

int EnemyNormal::s_modelHandle = -1;
bool EnemyNormal::s_shouldDrawCollision = false;
bool EnemyNormal::s_shouldDrawShieldCollision = false;

EnemyNormal::EnemyNormal()
    : m_headPosOffset(EnemyNormalConstants::kHeadShotPositionOffset)
    , m_hasTakenTackleDamage(false)
    , m_animTime(0.0f)
    , m_hasAttackHit(false)
    , m_onDropItem(nullptr)
    , m_currentAnimState(AnimState::Walk)
    , m_attackEndDelayTimer(0)
    , m_isDeadAnimPlaying(false)
    , m_hasDroppedItem(false)
    , m_voiceTimer(180 + GetRand(420))
    , m_distToPlayer(0.0f)
{
    // モデルの複製
    m_modelHandle = MV1DuplicateModel(s_modelHandle);

    // コライダーの初期化
    m_pBodyCollider = std::make_shared<CapsuleCollider>();
    m_pHeadCollider = std::make_shared<SphereCollider>();
    m_pAttackRangeCollider = std::make_shared<SphereCollider>();
    m_pAttackHitCollider = std::make_shared<CapsuleCollider>();
}

EnemyNormal::~EnemyNormal()
{
    // モデルの解放
    MV1DeleteModel(m_modelHandle);
}

void EnemyNormal::LoadModel()
{
    s_modelHandle = MV1LoadModel("data/model/NormalZombie.mv1");
    assert(s_modelHandle != -1);
}

void EnemyNormal::DeleteModel()
{
    MV1DeleteModel(s_modelHandle);
}

void EnemyNormal::Init()
{
    m_attackCooldownMax = EnemyNormalConstants::kAttackCooldownMax;

    m_isAlive = true;
    m_isDeadAnimPlaying = false; // フラグリセット
    m_hasDroppedItem = false;
    m_voiceTimer = 180 + GetRand(420); // 3秒〜10秒
    m_lastHitPart = HitPart::None; // 最後のヒット部位をリセット
    m_hitDisplayTimer = 0;         // ヒット表示タイマーもリセット
    m_damageTimer = 0;             // ダメージタイマー初期化
    m_isBlownAway = false;         // 吹き飛びフラグリセット
    m_deathKnockbackSpeed = 0.0f;  // 速度リセット

    // CSVからNormalEnemyのTransform情報を取得
    LoadTransformData("NormalEnemy");

    // ここで一度「絶対にWalkでない値」にリセット
    // 初期アニメーションを強制的に再生させるため
    m_currentAnimState = AnimState::Dead;

    // 初期ステートの設定
    ChangeState(std::make_shared<EnemyNormalStateWalk>());

    // ターゲットオフセットの初期化 (±100.0f)
    float offsetX = static_cast<float>(GetRand(200) - 100);
    float offsetZ = static_cast<float>(GetRand(200) - 100);
    m_targetOffset = VGet(offsetX, 0.0f, offsetZ);

    // 徘徊用パラメータ初期化
    m_wanderTimer = 0;
    m_wanderOffset = VGet(0.0f, 0.0f, 0.0f);

    // シールド関連の初期化フラグ
    m_hasShieldConfigured = false;
    m_isShieldBroken = false;
    m_shieldHp = 0.0f;
    m_maxShieldHp = 0.0f;
    m_shieldRotation = 0.0f;
    m_shieldEffectTimer = 0.0f;
    m_pShieldCollider = nullptr;
    m_shieldEffectHandles.clear();
    m_shieldChainBreakTimer = 0;
}

void EnemyNormal::SetHasShield(bool hasShield)
{
    m_hasShieldConfigured = hasShield;
    if (m_hasShieldConfigured)
    {
        m_maxShieldHp = 50.0f; // ノーマルゾンビ用シールド耐久値
        m_shieldHp = m_maxShieldHp;
        m_isShieldBroken = false;
        
        // シールドコライダーの生成と初期化 (ノーマルゾンビのサイズに合わせて調整)
        m_pShieldCollider = std::make_shared<SphereCollider>();
        m_pShieldCollider->SetRadius(80.0f); // ボディ(20)+α 程度の大きさ
        m_shieldRotation = 0.0f;
        m_shieldEffectTimer = 0.0f;
        m_shieldEffectHandles.clear();
        m_hasPlayedShieldBreakableEffect = false;
    }
}

// アニメーションを変更する
void EnemyNormal::ChangeAnimation(AnimState newAnimState, bool loop)
{
    if (m_currentAnimState == newAnimState)
    {
        if (newAnimState == AnimState::Attack)
        {
            // 同じ攻撃アニメーションをリセットして再開
            m_animationManager.PlayAnimation(m_modelHandle, EnemyNormalConstants::kAttackAnimName, loop);
        }
        else return;
    }

    const char* animName = nullptr;

    switch (newAnimState)
    {
    case AnimState::Walk:
        animName = EnemyNormalConstants::kWalkAnimName;
        break;
    case AnimState::Attack:
        animName = EnemyNormalConstants::kAttackAnimName;
        break;
    case AnimState::Damage: // ダメージ（怯み）時は歩行モーションなどを流用（あるいは専用）
        animName = EnemyNormalConstants::kWalkAnimName;
        break;
    case AnimState::Dead:
        animName = EnemyNormalConstants::kDeadAnimName;
        break;
    }

    if (animName)
    {
        // AnimationManagerにアニメーションの再生を依頼
        m_animationManager.PlayAnimation(m_modelHandle, animName, loop);
        m_animTime = 0.0f; // アニメーション切り替え時に時間をリセット
    }

    m_currentAnimState = newAnimState;
}

void EnemyNormal::ChangeState(std::shared_ptr<EnemyState<EnemyNormal>> newState)
{
    if (m_pCurrentState)
    {
        m_pCurrentState->Exit(this);
    }
    m_pCurrentState = newState;
    if (m_pCurrentState)
    {
        m_pCurrentState->Enter(this);
    }
}

// プレイヤーに攻撃可能かどうかを判定
bool EnemyNormal::CanAttackPlayer(const Player& player)
{
    int handRIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
    int handLIndex = MV1SearchFrame(m_modelHandle, "Hand_L");
    if (handRIndex == -1 || handLIndex == -1) return false;

    VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
    VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

    m_pAttackHitCollider->SetSegment(handRPos, handLPos);
    m_pAttackHitCollider->SetRadius(EnemyNormalConstants::kAttackHitRadius);

    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
    return m_pAttackHitCollider->IsIntersects(playerBodyCollider.get());
}

void EnemyNormal::Update(const EnemyUpdateContext& context)
{
    // コンテキストから必要な変数を展開
    std::vector<Bullet>& bullets = context.bullets;
    const Player::TackleInfo& tackleInfo = context.tackleInfo;
    const Player& player = context.player;
    const std::vector<EnemyBase*>& enemyList = context.enemyList;
    const std::vector<Stage::StageCollisionData>& collisionData = context.collisionData;
    Effect* pEffect = context.pEffect;

    // AI間引き処理の更新
    UpdateThrottling(player.GetPos());

    // 連鎖破壊タイマーの更新
    if (m_hasShieldConfigured && m_shieldChainBreakTimer > 0)
    {
        m_shieldChainBreakTimer--;
        if (m_shieldChainBreakTimer <= 0)
        {
            BreakShield(&context);
        }
    }

    // 視界外の単純動作モード
    if (m_isSimpleMode)
    {
        // 生存していれば簡易移動
        if (m_hp > 0.0f)
        {
            VECTOR playerPos = player.GetPos();
            VECTOR targetPos = VAdd(playerPos, m_targetOffset); // オフセット付き
            VECTOR toPlayer = VSub(targetPos, m_pos);
            toPlayer.y = 0.0f;
            float disToPlayer = VSize(toPlayer);

            if (disToPlayer > EnemyNormalConstants::kChaseStopDistance)
            {
                VECTOR dir = VNorm(toPlayer);
                float moveDist = disToPlayer - EnemyNormalConstants::kChaseStopDistance;
                float scaledSpeed = m_chaseSpeed * Game::GetTimeScale();
                float step = (std::min)(moveDist, scaledSpeed);
                m_pos.x += dir.x * step;
                m_pos.z += dir.z * step;

                // 向きも更新しておく (急に振り向くのを防ぐため、あるいは描画時に正しい向きにするため)
                float rotSpeed = 0.05f * Game::GetTimeScale();
                RotateTowards(targetPos, rotSpeed);
            }
        }

        // ステージとの当たり判定のみ簡易に行う（重力適用など）
        UpdateStageCollision(collisionData, context.collisionGrid);
        
        // モデルの位置更新
        MV1SetPosition(m_modelHandle, m_pos);
        return;
    }

    // ステージとの当たり判定
    UpdateStageCollision(collisionData, context.collisionGrid);

    // コライダーの更新 (間引き対象)
    // 遠方の敵は毎フレームボーン位置を取得する必要がないため、AI更新に同期させる
    if (m_shouldUpdateAI)
    {
        // 体のコライダー（カプセル）
        VECTOR bodyCapA = VAdd(m_pos, VGet(0, EnemyNormalConstants::kBodyColliderRadius, 0));
        VECTOR bodyCapB = VAdd(m_pos, VGet(0, EnemyNormalConstants::kBodyColliderHeight - EnemyNormalConstants::kBodyColliderRadius, 0));
        m_pBodyCollider->SetSegment(bodyCapA, bodyCapB);
        m_pBodyCollider->SetRadius(EnemyNormalConstants::kBodyColliderRadius);

        int headIndex = MV1SearchFrame(m_modelHandle, "Head");
        VECTOR headModelPos = (headIndex != -1)
                                  ? MV1GetFramePosition(m_modelHandle, headIndex)
                                  : VGet(0, 0, 0);
        VECTOR headCenter = VAdd(headModelPos, m_headPosOffset); // モデルの頭のフレーム位置にオフセットを適用
        m_pHeadCollider->SetCenter(headCenter);
        m_pHeadCollider->SetRadius(EnemyNormalConstants::kHeadRadius);

        // 攻撃範囲のコライダー（球）
        VECTOR attackRangeCenter = m_pos;
        attackRangeCenter.y += (EnemyNormalConstants::kBodyColliderHeight * 0.5f);
        m_pAttackRangeCollider->SetCenter(attackRangeCenter);
        m_pAttackRangeCollider->SetRadius(EnemyNormalConstants::kAttackRangeRadius);

        // シールドの更新処理
        if (m_hasShieldConfigured && !m_isShieldBroken && m_pShieldCollider)
        {
            // 衝突判定用に現在のシールド位置を計算
            VECTOR shieldPos = m_pos;
            shieldPos.y += EnemyNormalConstants::kBodyColliderHeight * 0.5f; // 胸のあたり

            m_pShieldCollider->SetCenter(shieldPos);
        }
    }


    // シールドの更新処理
    if (m_hasShieldConfigured && !m_isShieldBroken && m_pShieldCollider)
    {
        // 衝突判定用に現在のシールド位置を計算
        VECTOR shieldPos = m_pos;
        shieldPos.y += EnemyNormalConstants::kBodyColliderHeight * 0.5f; // 胸のあたり

        m_pShieldCollider->SetCenter(shieldPos);

        // シールドエフェクト表示処理 (生存時のみ)
        if (m_hp > 0.0f)
        {
            // エフェクト生成ロジック
            // フェードイン30F, 総再生240F を想定 -> 210Fで次を生成して重ねる
            const float kEffectDuration = 240.0f;
            const float kFadeInDuration = 30.0f; 
            const float kOverlapSpawnTime = kEffectDuration - kFadeInDuration;

            // タイマー更新
            m_shieldEffectTimer += 1.0f * Game::GetTimeScale();

            // エフェクトがない、または再生時間が重なり開始時間を超えたら新規生成
            if (m_shieldEffectHandles.empty() || m_shieldEffectTimer >= kOverlapSpawnTime)
            {
                if (pEffect)
                {
                    // バリアエフェクト再生
                    int handle = pEffect->PlayBossShieldEffect(shieldPos.x, shieldPos.y, shieldPos.z);
                    if (handle != -1)
                    {
                        m_shieldEffectHandles.push_back(handle);
                        m_shieldEffectTimer = 0.0f; // 次の生成までの時間をリセット
                    }
                }
            }

            // 回転と位置の更新
            m_shieldRotation += 0.3f * Game::GetTimeScale();
            while (m_shieldRotation >= 360.0f) m_shieldRotation -= 360.0f;

            auto it = m_shieldEffectHandles.begin();
            while (it != m_shieldEffectHandles.end())
            {
                int handle = *it;
                if (IsEffekseer3DEffectPlaying(handle) == -1)
                {
                    // 再生終了していたら削除
                    it = m_shieldEffectHandles.erase(it);
                    continue;
                }

                SetPosPlayingEffekseer3DEffect(handle, shieldPos.x, shieldPos.y, shieldPos.z);
                SetRotationPlayingEffekseer3DEffect(handle, 0.0f, (m_shieldRotation * DX_PI_F / 180.0f), 0.0f);
                
                // ノーマルゾンビ用なのでボスのシールドより小さめにする (ベーススケールを当たり判定に合うように0.3程度に設定・HPで縮小しない)
                SetScalePlayingEffekseer3DEffect(handle, 0.3f, 0.3f, 0.3f);

                // シールドの色変更 (青 -> 赤)
                if (m_maxShieldHp > 0.0f)
                {
                    float ratio = m_shieldHp / m_maxShieldHp;
                    if (ratio < 0.0f) ratio = 0.0f;
                    if (ratio > 1.0f) ratio = 1.0f;

                    int r = static_cast<int>(255.0f * (1.0f - ratio)); // hp減少(ratioが0に近づく)で赤くなる
                    int g = 0;
                    int b = static_cast<int>(255.0f * ratio);          // hp最大(ratioが1)で青くなる
                    
                    SetColorPlayingEffekseer3DEffect(handle, r, g, b, 255);
                }

                ++it;
            }
        }
    }

    // プレイヤーとの距離を更新
    m_distToPlayer = VSize(VSub(context.player.GetPos(), m_pos));

    // 環境ボイスの更新
    if (m_isAlive && !m_isDeadAnimPlaying)
    {
        m_voiceTimer--;
        if (m_voiceTimer <= 0)
        {
            int randomIndex = 1 + GetRand(3); // 1〜4
            
            float maxDist = 2000.0f;
            float volRatio = 1.0f - (m_distToPlayer / maxDist);
            if (volRatio < 0.0f) volRatio = 0.0f;
            if (volRatio > 1.0f) volRatio = 1.0f;

            SoundManager::GetInstance()->Play("EnemyNormal", "Voice" + std::to_string(randomIndex), (int)(150 * volRatio));

            // 次の再生までの時間をランダムに設定 (3秒〜10秒)
            m_voiceTimer = 180 + GetRand(420);
        }
    }

    if (m_hp <= 0.0f)
    {
        // 死亡時にシールドエフェクトが残存している場合は破棄する
        if (!m_shieldEffectHandles.empty())
        {
            for (int handle : m_shieldEffectHandles)
            {
                StopEffekseer3DEffect(handle);
            }
            m_shieldEffectHandles.clear();
            m_pShieldCollider = nullptr;
        }

        UpdateDeath(collisionData);
        return;
    }

    MV1SetPosition(m_modelHandle, m_pos); // モデルの位置は常に反映する

    // 攻撃アニメーション中は追尾しない
    if (m_currentAnimState == AnimState::Walk) // 追尾はWalk状態でのみ行う
    {
        VECTOR playerPos = player.GetPos();
        VECTOR targetPos;

        // プレイヤーが岩の上にいる場合は、プレイヤーの周囲をうろうろする
        std::string groundObj = player.GetGroundedObjectName();
        if (groundObj == "Rock3" || groundObj == "Rock6")
        {
            m_wanderTimer--;
            if (m_wanderTimer <= 0)
            {
                // 2秒ごとに新しい目標位置を設定 (距離300~700) - 範囲拡大
                m_wanderTimer = 120;
                float angle = static_cast<float>(GetRand(360)) * DX_PI_F / 180.0f;
                float dist = static_cast<float>(300 + GetRand(400));
                m_wanderOffset = VGet(cosf(angle) * dist, 0.0f, sinf(angle) * dist);
            }
            targetPos = VAdd(playerPos, m_wanderOffset);
        }
        else
        {
            // 通常時はプレイヤーに向かう（オフセット付き）
            targetPos = VAdd(playerPos, m_targetOffset);
        }

        VECTOR toPlayer = VSub(targetPos, m_pos);
        toPlayer.y = 0.0f; // Y成分を無視して水平距離を計算

        // プレイヤー(ターゲット)との距離を計算
        float disToPlayer = VSize(toPlayer);

        // 補間速度(0.05f で滑らかにする)
        float rotSpeed = 0.05f * Game::GetTimeScale();
        RotateTowards(targetPos, rotSpeed);

        // 移動処理
        if (disToPlayer > EnemyNormalConstants::kChaseStopDistance)
        {
            VECTOR dir = VNorm(toPlayer);
            float moveDist = disToPlayer - EnemyNormalConstants::kChaseStopDistance;
            // タイムスケールを移動速度に適用
            float scaledSpeed = m_chaseSpeed * Game::GetTimeScale();
            float step = (std::min)(moveDist, scaledSpeed); // 1フレームで進みすぎない
            m_pos.x += dir.x * step;
            m_pos.z += dir.z * step;
        }
    }

    // プレイヤーのカプセルコライダー情報を取得
    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();
    bool isPlayerInAttackRange = m_pAttackRangeCollider->IsIntersects(playerBodyCollider.get());

    // AIステートの更新
    if (m_pCurrentState)
    {
        m_pCurrentState->Update(this, context);
    }

    // アニメーションがアタッチされている場合のみ時間を更新 (AI間引き対象)
    if (m_shouldUpdateAI && m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
    {
        m_animTime += (1.0f * m_aiUpdateInterval) * Game::GetTimeScale();

        const char* animName = nullptr;
        if (m_currentAnimState == AnimState::Walk || m_currentAnimState == AnimState::Damage)
        {
            animName = EnemyNormalConstants::kWalkAnimName;
        }
        else if (m_currentAnimState == AnimState::Attack)
        {
            animName = EnemyNormalConstants::kAttackAnimName;
        }
        else
        {
            animName = EnemyNormalConstants::kDeadAnimName;
        }

        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, animName);

        if (m_currentAnimState == AnimState::Attack)
        {
            // ここは何もしない
        }
        else if (m_currentAnimState == AnimState::Dead)
        {
            if (m_animTime >= currentAnimTotalTime)
            {
                m_animTime = currentAnimTotalTime;
            }
        }
        else if (m_currentAnimState == AnimState::Walk || m_currentAnimState == AnimState::Damage)
        {
            if (m_animTime >= currentAnimTotalTime)
            {
                m_animTime = fmodf(m_animTime, currentAnimTotalTime);
            }
        }
        // AnimationManagerにアニメーション時間の更新を依頼
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }

    // 敵とプレイヤーの押し出し処理（カプセル同士の衝突）
    float minDist = EnemyNormalConstants::kBodyColliderRadius + playerBodyCollider->GetRadius(); // 最小距離は両方の半径の和
    ResolvePlayerCollision(playerBodyCollider, minDist, EnemyNormalConstants::kPushBackEpsilon);

    // シールドとの押し出し処理
    if (m_hasShieldConfigured && !m_isShieldBroken && m_pShieldCollider)
    {
        float shieldRadius = m_pShieldCollider->GetRadius();
        
        // プレイヤーの位置を取得
        if (Game::m_pPlayer)
        {
            VECTOR playerPos = Game::m_pPlayer->GetPos();
            VECTOR playerCapA, playerCapB;
            float playerRadius;
            Game::m_pPlayer->GetCapsuleInfo(playerCapA, playerCapB, playerRadius);

            CapsuleCollider playerCol(playerCapA, playerCapB, playerRadius);

            // 球(シールド) と カプセル(プレイヤー) の交差判定
            if (m_pShieldCollider->IsIntersects(&playerCol))
            {
                // ここでは単純に押し出し（ボスと同じ実装を利用）
                VECTOR diff = VSub(playerPos, m_pShieldCollider->GetCenter());
                diff.y = 0.0f; 
                float distSq = VSquareSize(diff);
                float minDistWithShield = shieldRadius + playerRadius;

                if (distSq < minDistWithShield * minDistWithShield && distSq > EnemyNormalConstants::kPushBackEpsilon)
                {
                    float dist = sqrtf(distSq);
                    VECTOR pushDir = VScale(diff, 1.0f / dist);
                    float pushDist = minDistWithShield - dist;

                    Game::m_pPlayer->SetPos(VAdd(playerPos, VScale(pushDir, pushDist)));
                }
            }
        }
    }

    // 敵同士の押し出し処理 (間引き対象)
    if (m_shouldUpdateAI)
    {
        std::vector<EnemyBase*> neighbors;
        if (context.collisionGrid)
        {
            context.collisionGrid->GetNeighbors(m_pos, neighbors);
        }
        const std::vector<EnemyBase*>& targets = (context.collisionGrid) ? neighbors : enemyList;

        ResolveEnemyCollision(targets, EnemyNormalConstants::kBodyColliderRadius, EnemyNormalConstants::kPushBackEpsilon);
    }

    if (m_currentAnimState == AnimState::Attack) // 攻撃アニメーションが再生中の場合のみ攻撃判定を行う
    {
        // m_shouldUpdateAI
        // で判定して、間引き時は前回の結果を維持（あるいはスキップ）
        // ここでは攻撃の当たり判定は重いので間引き対象にする
        if (m_shouldUpdateAI)
        {
            // m_currentAnimTotalTime を AnimationManager から取得
            float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyNormalConstants::kAttackAnimName);
            float attackStart = currentAnimTotalTime * 0.5f; // 攻撃開始時間
            float attackEnd = currentAnimTotalTime * 0.7f;   // 攻撃終了時間

            // 攻撃アニメーションの範囲内でのみ攻撃判定を行う
            if (!m_hasAttackHit && m_animTime >= attackStart && m_animTime <= attackEnd)
            {
                int handRIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
                int handLIndex = MV1SearchFrame(m_modelHandle, "Hand_L");
                if (handRIndex != -1 && handLIndex != -1)
                {
                    VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
                    VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

                    // 攻撃ヒット用コライダーの更新
                    m_pAttackHitCollider->SetSegment(handRPos, handLPos);
                    m_pAttackHitCollider->SetRadius(EnemyNormalConstants::kAttackHitRadius);

                    if (m_pAttackHitCollider->IsIntersects(playerBodyCollider.get()))
                    {
                        const_cast<Player&>(player).TakeDamage(m_attackPower, m_pos); // プレイヤーにダメージ（攻撃者の位置を渡す）
                        m_hasAttackHit = true;
                    }
                }
            }
        }
    }

    CheckHitAndDamage(const_cast<std::vector<Bullet>&>(bullets), pEffect);

    if (tackleInfo.isTackling && m_hp > 0.0f && tackleInfo.tackleId != m_lastTackleId)
    {
        CapsuleCollider playerTackleCollider(tackleInfo.capA, tackleInfo.capB, tackleInfo.radius);

        if (m_pBodyCollider->IsIntersects(&playerTackleCollider))
        {
            TakeTackleDamage(tackleInfo.damage);
            m_lastTackleId = tackleInfo.tackleId;
        }
    }
    else if (!tackleInfo.isTackling)
    {
        m_lastTackleId = -1;
    }

    if (m_hitDisplayTimer > 0)
    {
        --m_hitDisplayTimer;
        if (m_hitDisplayTimer == 0)
        {
            m_lastHitPart = HitPart::None;
        }
    }
}

void EnemyNormal::Draw()
{
    // 死亡アニメーション終了後も描画しない
    if (m_hp <= 0.0f && m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) == -1) return;

    // 視錐台カリング (描画最適化)
    if (!ShouldDraw(EnemyNormalConstants::kDrawDistanceSq, EnemyNormalConstants::kDrawNearDistanceSq, EnemyNormalConstants::kDrawDotThreshold)) return;

    EnemyBase::IncrementDrawCount();
    MV1DrawModel(m_modelHandle);

    if (s_shouldDrawCollision || s_shouldDrawShieldCollision)
    {
        DrawCollisionDebug();
    }

#ifdef _DEBUG
    const char* hitMsg = "";

    switch (m_lastHitPart)
    {
    case HitPart::Head:
        hitMsg = "ヘッドショット！";
        break;
    case HitPart::Body:
        hitMsg = "ボディヒット！";
        break;
    default:
        break;
    }

    if (*hitMsg)
    {
        DrawFormatString(20, 100, 0xff0000, "%s", hitMsg);
    }
#endif
}

// デバック用の当たり判定を描画する
void EnemyNormal::DrawCollisionDebug() const
{
    if (s_shouldDrawShieldCollision && m_hasShieldConfigured && !m_isShieldBroken && m_pShieldCollider)
    {
        DebugUtil::DrawSphere(m_pShieldCollider->GetCenter(), m_pShieldCollider->GetRadius(), 16, 0x00ffff);
    }

    if (!s_shouldDrawCollision) return;

    // 体のコライダーデバッグ描画
    DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB(), m_pBodyCollider->GetRadius(), 16, 0xff0000);

    // 頭のコライダーデバッグ描画
    DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(), m_pHeadCollider->GetRadius(), 16, 0x00ff00);

    // 攻撃範囲のデバッグ描画
    DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetRadius(), 24, 0xff8000);

    int handRIndex = MV1SearchFrame(m_modelHandle, "Hand_R");
    int handLIndex = MV1SearchFrame(m_modelHandle, "Hand_L");

    if (handRIndex != -1 && handLIndex != -1)
    {
        VECTOR handRPos = MV1GetFramePosition(m_modelHandle, handRIndex);
        VECTOR handLPos = MV1GetFramePosition(m_modelHandle, handLIndex);

        // 攻撃ヒット用コライダーのデバッグ描画
        DebugUtil::DrawCapsule(handRPos, handLPos, EnemyNormalConstants::kAttackHitRadius, 16, 0x0000ff);
    }
}

// どこに当たったのか判定する
EnemyBase::HitPart EnemyNormal::CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
    if (m_isDeadAnimPlaying) return HitPart::None;

    // シールドヒット判定
    if (m_hasShieldConfigured && !m_isShieldBroken && m_pShieldCollider)
    {
        VECTOR hitPosShield;
        float hitDistSqShield;
        if (m_pShieldCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosShield, hitDistSqShield))
        {
            outHtPos = hitPosShield;
            outHtDistSq = hitDistSqShield;
            return HitPart::Shield;
        }
    }

    // ヘッドとボディのコライダーをそれぞれチェック
    VECTOR hitPosHead, hitPosBody;
    float hitDistSqHead = FLT_MAX;
    float hitDistSqBody = FLT_MAX;

    // ヘッドとボディのコライダーに対してRayをチェック
    bool headHit = m_pHeadCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosHead, hitDistSqHead);
    bool bodyHit = m_pBodyCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosBody, hitDistSqBody);

    // ヒットした部位を判定
    if (headHit && bodyHit)
    {
        // 両方にヒットした場合、より近い方を優先
        if (hitDistSqHead <= hitDistSqBody)
        {
            outHtPos = hitPosHead;
            outHtDistSq = hitDistSqHead;
            return HitPart::Head;
        }
        else
        {
            outHtPos = hitPosBody;
            outHtDistSq = hitDistSqBody;
            return HitPart::Body;
        }
    }
    else if (headHit)
    {
        outHtPos = hitPosHead;
        outHtDistSq = hitDistSqHead;
        return HitPart::Head;
    }
    else if (bodyHit)
    {
        outHtPos = hitPosBody;
        outHtDistSq = hitDistSqBody;
        return HitPart::Body;
    }

    outHtPos = VGet(0, 0, 0); // ヒットしない場合は適当な値を入れておく
    outHtDistSq = FLT_MAX;
    return HitPart::None;
}

// アイテムドロップ時のコールバック関数
void EnemyNormal::SetOnDropItemCallback(std::function<void(const VECTOR&)> cb)
{
    m_onDropItem = cb;
}

// ダメージ処理
void EnemyNormal::TakeDamage(float damage, AttackType type)
{
    if (m_isDeadAnimPlaying) return;

    // シールド処理
    if (m_hasShieldConfigured && !m_isShieldBroken)
    {
        // シールドHPが0以下の状態で盾投げを食らったら破壊
        if (m_shieldHp <= 0.0f && type == AttackType::ShieldThrow)
        {
            BreakShield(); 
            TriggerShieldChainBreak(1); // 次のUpdateで連鎖を開始させる
        }
        else if (type != AttackType::ShieldThrow) // 盾投げ以外はシールドHPを減らす
        {
            m_shieldHp -= damage;
            if (m_shieldHp < 0.0f)
            {
                m_shieldHp = 0.0f;
            }

            // シールドHPが0になった瞬間に、破壊可能エフェクトを再生
            if (m_shieldHp <= 0.0f && !m_hasPlayedShieldBreakableEffect)
            {
                if (m_pShieldCollider)
                {
                    // シールドの位置で再生
                    SceneMain::Instance()->GetEffect()->PlayShieldBreakEffect(m_pShieldCollider->GetCenter());
                }
                m_hasPlayedShieldBreakableEffect = true;
            }
        }
        
        // シールドがある間は本体にダメージを与えない
        return;
    }

    EnemyBase::TakeDamage(damage, type);

    // 怯み処理
    if (m_hp > 0.0f)
    {
        if (type == AttackType::Shotgun)
        {
            // ショットガンは上書きしてでも大きく怯む
            ChangeState(std::make_shared<EnemyNormalStateDamage>());
            m_damageTimer = EnemyNormalConstants::kDamageDuration; // 30フレーム
        }
        else if (type == AttackType::Shoot)
        {
            // アサルトライフルは短い怯み（連射によるスタンロック防止のため、既に怯み中なら上書きしない）
            if (m_currentAnimState != AnimState::Damage)
            {
                ChangeState(std::make_shared<EnemyNormalStateDamage>());
                m_damageTimer = 15; // ショットガンの半分の時間（15フレーム）
            }
        }
    }
}

void EnemyNormal::OnDeath()
{
    if (m_lastAttackType == AttackType::Shotgun)
    {
        m_isBlownAway = true;
        // プレイヤーと逆方向に吹き飛ぶ
        if (SceneMain::Instance())
        {
            VECTOR playerPos = SceneMain::Instance()->GetPlayer().GetPos();
            VECTOR toEnemy = VSub(m_pos, playerPos);
            toEnemy.y = 0.0f; // 水平方向のみ
            if (VSquareSize(toEnemy) > EnemyNormalConstants::kPushBackEpsilon)
            {
                m_deathKnockbackDir = VNorm(toEnemy);
                m_deathKnockbackSpeed = 15.0f; // 初速を強化
            }
        }

        // フォールバック: もし速度が設定されなかった場合（プレイヤー位置取得失敗 or 重なり）
        if (m_deathKnockbackSpeed <= 0.0f)
        {
            // 敵の向いている方向の逆（後ろ）へ飛ばす
            MATRIX worldMat = MV1GetLocalWorldMatrix(m_modelHandle);
            VECTOR forward = VGet(worldMat.m[2][0], worldMat.m[2][1], worldMat.m[2][2]);
            forward.y = 0.0f;
            if (VSquareSize(forward) > EnemyNormalConstants::kPushBackEpsilon)
            {
                m_deathKnockbackDir = VScale(VNorm(forward), -1.0f);
            }
            else
            {
                m_deathKnockbackDir = VGet(0, 0, 1); // 完全なフォールバック
            }
            m_deathKnockbackSpeed = 15.0f;
        }
    }

    if (m_lastHitPart == HitPart::None)
        m_lastHitPart = HitPart::Body;
    bool isHeadShot = (m_lastHitPart == HitPart::Head);
    int addScore = ScoreManager::Instance().AddScore(isHeadShot);
    if (SceneMain::Instance())
    {
        SceneMain::Instance()->AddScorePopup(addScore, isHeadShot, ScoreManager::Instance().GetCombo());
    }
}

// タックル攻撃のダメージ処理
void EnemyNormal::TakeTackleDamage(float damage)
{
    if (m_isDeadAnimPlaying) return;

    EnemyBase::TakeTackleDamage(damage);
}

std::shared_ptr<CapsuleCollider> EnemyNormal::GetBodyCollider() const
{
    return m_pBodyCollider;
}

// 弾との当たり判定とダメージ処理 (複数弾ヒットや貫通対策用)
void EnemyNormal::CheckHitAndDamage(std::vector<Bullet>& bullets, Effect* pEffect)
{
    // ショットガン等の散弾や複数弾が同時に当たるケースを想定
    // ループを1回にまとめて効率化

    for (auto& bullet : bullets)
    {
        if (!bullet.IsActive()) continue;

        VECTOR rayStart = bullet.GetPrevPos();
        VECTOR rayEnd = bullet.GetPos();

        // 共通の距離チェック（ブロードフェーズ）
        // EnemyBase::FindClosestHitBullet でも行っているが、ここでは独自に判定が必要なため
        VECTOR enemyCenter = VAdd(m_pos, VGet(0, EnemyNormalConstants::kBodyColliderHeight * 0.5f, 0));
        float d1 = VSquareSize(VSub(rayStart, enemyCenter));
        float d2 = VSquareSize(VSub(rayEnd, enemyCenter));
        if (d1 > 300.0f * 300.0f && d2 > 300.0f * 300.0f) continue;

        // ① シールド判定 (シールドがある場合)
        if (m_hasShieldConfigured && !m_isShieldBroken && m_pShieldCollider)
        {
            VECTOR hitPosShield;
            float hitDistSqShield;
            if (m_pShieldCollider->IsIsIntersectsRay(rayStart, rayEnd, hitPosShield, hitDistSqShield))
            {
                ApplyBulletDamage(bullet, HitPart::Shield, hitDistSqShield, pEffect);
                continue; // シールドに当たったら本体判定は行わない
            }
        }

        // ② 本体判定
        VECTOR hitPos;
        float hitDistSq;
        HitPart part = CheckHitPart(rayStart, rayEnd, hitPos, hitDistSq);
        if (part != HitPart::None)
        {
            ApplyBulletDamage(bullet, part, hitDistSq, pEffect);
        }
    }
}

// ダメージ計算
float EnemyNormal::CalcDamage(float bulletDamage, HitPart part) const
{
    if (m_isDeadAnimPlaying) return 0.0f;

    // シールドヒット時
    if (part == HitPart::Shield && m_hasShieldConfigured && !m_isShieldBroken)
    {
        return bulletDamage; // 等倍ダメージとする
    }

    // ヘッドショット時
    if (part == HitPart::Head)
    {
        return bulletDamage * 2.0f; // 2倍ダメージ
    }

    // それ以外（Bodyなど）
    return bulletDamage * 1.0f;
}

// ダメージ適用（シールドヒット時はエフェクトを変えるためオーバーライド）
void EnemyNormal::ApplyBulletDamage(Bullet& bullet, HitPart part, float distSq, Effect* pEffect)
{
    // シールドヒット時
    if (part == HitPart::Shield && m_hasShieldConfigured && !m_isShieldBroken)
    {
        // シールド専用エフェクト (HitBurst)
        if (pEffect)
        {
            VECTOR hitPos = bullet.GetPos();
            VECTOR normal = VGet(0.0f, 0.0f, -1.0f); // デフォルト

            if (m_pShieldCollider)
            {
                VECTOR center = m_pShieldCollider->GetCenter();
                VECTOR diff = VSub(hitPos, center);
                if (VSquareSize(diff) > 0.0001f)
                {
                    normal = VNorm(diff);
                    // 弾の現在位置(内側にめり込んでいる可能性がある)ではなく、シールドの表面でエフェクトを発生させる
                    hitPos = VAdd(center, VScale(normal, m_pShieldCollider->GetRadius()));
                }
            }

            pEffect->PlayShieldHitEffect(hitPos, normal);
        }

        // ダメージ計算と適用 (シールドHP減少)
        float damage = CalcDamage(bullet.GetDamage(), part);
        TakeDamage(damage, bullet.GetAttackType()); // TakeDamage側で減算処理を行う

        // デバッグ表示用
        if (s_shouldShowDamage)
        {
            s_debugLastDamage = damage;
            s_debugDamageTimer = EnemyConstants::kDebugDamageDisplayTimer;
            s_debugHitInfo = "(Shield)";
        }

        // 弾を非アクティブ化する
        bullet.Deactivate();
    }
    else
    {
        // それ以外は基底クラス（出血エフェクトあり）
        EnemyBase::ApplyBulletDamage(bullet, part, distSq, pEffect);
    }
}

void EnemyNormal::TriggerShieldChainBreak(int delayFrames)
{
    if (!m_hasShieldConfigured) return;
    // 既に破壊されていても、連鎖の起点になるためにタイマー設定を許可する
    m_shieldChainBreakTimer = delayFrames;
}

void EnemyNormal::BreakShield(const EnemyUpdateContext* context)
{
    if (!m_hasShieldConfigured) return;

    if (!m_isShieldBroken)
    {
        m_isShieldBroken = true;
        m_pShieldCollider = nullptr;

        // 再生中のエフェクト停止
        for (int handle : m_shieldEffectHandles)
        {
            StopEffekseer3DEffect(handle);
        }
        m_shieldEffectHandles.clear();

        // 破壊エフェクト再生
        if (SceneMain::Instance() && SceneMain::Instance()->GetEffect())
        {
            VECTOR shieldPos = m_pos;
            shieldPos.y += EnemyNormalConstants::kBodyColliderHeight * 0.5f;
            SceneMain::Instance()->GetEffect()->PlayShieldBreakEffect(shieldPos);

            // 画面揺れ演出の追加
            if (Game::m_pPlayer && Game::m_pPlayer->GetCamera())
            {
                // プレイヤーとの距離に応じて揺れの強さを変える
                float dist = VSize(VSub(m_pos, Game::m_pPlayer->GetPos()));
                float maxDist = 1000.0f;
                float intensityBase = 20.0f; // 基本の揺れ強度
                
                float ratio = 1.0f - (dist / maxDist);
                if (ratio < 0.2f) ratio = 0.2f; // 遠くても最低限は揺らす
                if (ratio > 1.0f) ratio = 1.0f;

                Game::m_pPlayer->GetCamera()->Shake(intensityBase * ratio, 10);
            }
        }
    }

    // 周囲の敵への連鎖 (contextがある場合のみ)
    if (context)
    {
        float chainRadius = 400.0f; // 連鎖範囲 (250から拡大)
        std::vector<EnemyBase*> neighbors;
        if (context->collisionGrid)
        {
            context->collisionGrid->GetNeighbors(m_pos, neighbors);
        }
        else
        {
            neighbors = context->enemyList;
        }

        for (auto* enemyBase : neighbors)
        {
            auto* normalEnemy = dynamic_cast<EnemyNormal*>(enemyBase);
            if (normalEnemy && normalEnemy != this && normalEnemy->m_hasShieldConfigured && !normalEnemy->m_isShieldBroken)
            {
                float distSq = VSquareSize(VSub(m_pos, normalEnemy->GetPos()));
                if (distSq < chainRadius * chainRadius)
                {
                    // まだ連鎖タイマーが動いていない場合のみセット
                    if (normalEnemy->m_shieldChainBreakTimer <= 0)
                    {
                        normalEnemy->TriggerShieldChainBreak(12); // 12フレーム（約0.2秒）の遅延
                    }
                }
            }
        }
    }
}

// 死亡時の更新処理
void EnemyNormal::UpdateDeath(const std::vector<Stage::StageCollisionData>& collisionData)
{
    if (!m_isDeadAnimPlaying)
    {
        // スコア加算処理はTakeDamageで行うのでここでは不要
        ChangeState(std::make_shared<EnemyNormalStateDead>());
        m_isDeadAnimPlaying = true;
        m_animTime = 0.0f; // アニメーション時間をリセット
        m_isAlive = true;  // 死亡アニメーション中はtrueのまま
    }

    // 死亡アニメーション中もアニメーション時間を更新
    if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
    {
        if (m_shouldUpdateAI)
        {
            m_animTime += (1.0f * m_aiUpdateInterval) * Game::GetTimeScale();
            m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
        }
    }

    // 死亡吹き飛び処理
    if (m_isBlownAway && m_deathKnockbackSpeed > 0.0f)
    {
        // 移動
        m_pos = VAdd(m_pos, VScale(m_deathKnockbackDir, m_deathKnockbackSpeed * Game::GetTimeScale()));
        // 減速 (摩擦)
        m_deathKnockbackSpeed -= 0.5f * Game::GetTimeScale();
        if (m_deathKnockbackSpeed < 0.0f) m_deathKnockbackSpeed = 0.0f;

        // ステージ衝突判定 (壁抜け防止)
        UpdateStageCollision(collisionData);
    }

    // モデル位置更新 (死亡中も移動するため必要)
    MV1SetPosition(m_modelHandle, m_pos);

    float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyNormalConstants::kDeadAnimName);
    if (m_animTime >= currentAnimTotalTime)
    {
        if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
        {
            MV1DetachAnim(m_modelHandle, 0);
            m_animationManager.ResetAttachedAnimHandle(m_modelHandle);
        }
        // アイテムドロップと死亡コールバックを呼び出し
        if (!m_hasDroppedItem && m_onDropItem)
        {
            m_onDropItem(m_pos);
            m_onDropItem = nullptr;
            m_hasDroppedItem = true;
        }
        if (m_onDeathCallback)
        {
            m_onDeathCallback(m_pos);
            m_onDeathCallback = nullptr; // 一度だけ呼び出す
        }
        m_isAlive = false; // 死亡アニメーション終了時のみfalseにする
        SetActive(false);  // プールに戻す
    }
}