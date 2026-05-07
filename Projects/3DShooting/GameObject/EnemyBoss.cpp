#include "EnemyBoss.h"
#include "Bullet.h"
#include "CapsuleCollider.h"
#include "Collision.h"
#include "DebugUtil.h"
#include "DxLib.h"
#include "Effect.h"
#include "EffekseerForDXLib.h"
#include "SceneMain.h"
#include "ScoreManager.h"
#include "SphereCollider.h"
#include "TaskTutorialManager.h"
#include "TransformDataLoader.h"
#include "Game.h"
#include "CollisionGrid.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include "EnemyBossState.h"

namespace EnemyBossConstants
{
    // アニメーション名
    constexpr char kWalkAnimName[]            = "Armature|Run";
    constexpr char kCloseAttackAnimName[]     = "Armature|CloseRangeAttack"; // 近接範囲攻撃
    constexpr char kLongRangeAttackAnimName[] = "Armature|LongRangeAttack";  // 遠距離攻撃
    constexpr char kDeadAnimName[]            = "Armature|Death";

    constexpr float kLongRangeAttackMinDist   = 400.0f; // 遠距離攻撃を行う最小距離
    constexpr float kLongRangeAttackMaxDist   = 1000.0f; // 遠距離攻撃を行う最大距離（これより遠いと攻撃せず接近する）
    constexpr int kLongRangeAttackCooldownMax = 120;
    constexpr float kHomingBulletSpeed        = 6.0f;
    constexpr float kHomingTurnRate           = 0.02f;        // 旋回性能
    constexpr float kHomingBulletMaxDist      = 1800.0f; // 弾の最大飛距離
    constexpr float kHomingBulletDamage       = 20.0f;
    constexpr float kHomingBulletRadius       = 15.0f;

    // コライダーサイズ
    constexpr float kBodyColliderRadius = 40.0f;
    constexpr float kBodyColliderHeight = 350.0f;
    constexpr float kHeadRadius         = 25.0f;
    constexpr float kAttackRangeRadius  = 450.0f; // 指定された近接範囲 
    constexpr float kAttackHitRadius    = 60.0f;    // 攻撃自体の当たり判定

	// 攻撃関連
    constexpr int kAttackCooldownMax = 60;
    constexpr int kAttackEndDelay    = 30; // 攻撃後の硬直

    // 描画関連
    constexpr float kDrawDistanceSq     = 10000.0f * 10000.0f; // 16000から10000に縮小
    constexpr float kDrawNearDistanceSq = 600.0f * 600.0f;

	// シールド関連
	constexpr float kShieldMaxHp = 200.0f; // シールドの最大耐久値
}

// static変数の初期化
int EnemyBoss::s_modelHandle = -1;
bool EnemyBoss::s_shouldDrawCollision = false;
bool EnemyBoss::s_shouldDrawAttackHit = false;
bool EnemyBoss::s_shouldDrawShieldCollision = false;

void EnemyBoss::LoadModel()
{
    s_modelHandle = MV1LoadModel("data/model/Boss.mv1");
    assert(s_modelHandle != -1);
}

void EnemyBoss::DeleteModel()
{
    MV1DeleteModel(s_modelHandle);
    s_modelHandle = -1;
}

EnemyBoss::EnemyBoss()
    : m_currentAnimState(AnimState::Idle)
    , m_isDeadAnimPlaying(false)
    , m_animTime(0.0f)
    , m_attackEndDelayTimer(0)
    , m_hasAttackHit(false)
    , m_currentEffectHandle(-1)
    , m_effectTimer(0)
    , m_hasPlayedCloseRangeEffect(false)
	, m_maxShieldHp(EnemyBossConstants::kShieldMaxHp)
	, m_shieldHp(EnemyBossConstants::kShieldMaxHp)
	, m_isShieldBroken(false)
	, m_isStunned(false)
	, m_stunTimer(0)
	, m_isFirstUpdate(true)
	, m_hasShotLongRange(false)
	, m_isNextAttackNormal(false)
	, m_hasPlayedShieldBreakableEffect(false)
	, m_shouldDrawParryCollider(false)
	, m_shieldRotation(0.0f)
	, m_shieldEffectTimer(0.0f)
	, m_headNodeIndex(-1)
	, m_headTopEndNodeIndex(-1)
	, m_pShieldCollider(nullptr)
	, m_pBodyCollider(nullptr)
	, m_pHeadCollider(nullptr)
	, m_pAttackRangeCollider(nullptr)
	, m_pAttackHitCollider(nullptr)
	, m_longRangeAttackCooldown(0)
{
    m_modelHandle = MV1DuplicateModel(s_modelHandle);

    // コライダー初期化
    m_pBodyCollider = std::make_shared<CapsuleCollider>();
    m_pHeadCollider = std::make_shared<SphereCollider>();
    m_pAttackRangeCollider = std::make_shared<SphereCollider>();
    m_pAttackHitCollider = std::make_shared<CapsuleCollider>();
}

EnemyBoss::~EnemyBoss()
{
    MV1DeleteModel(m_modelHandle);
}

void EnemyBoss::Init()
{
    m_isAlive = true;
    m_isDeadAnimPlaying = false;
    m_animTime = 0.0f;

    // CSVからBossのTransform情報を取得
    bool loadResult = LoadTransformData("Boss");

    // 基本パラメータ初期化
    m_hasAttackHit = false;
    m_attackEndDelayTimer = 0;
    m_attackCooldown = 0;
    m_hitDisplayTimer = 0;

    // フレームインデックスのキャッシュ
    m_headNodeIndex = MV1SearchFrame(m_modelHandle, "Head");
    m_headTopEndNodeIndex = MV1SearchFrame(m_modelHandle, "mixamorig:HeadTop_End");

    // 攻撃範囲コライダー設定
    m_pAttackRangeCollider->SetRadius(EnemyBossConstants::kAttackRangeRadius);

    // シールドコライダー設定 (初期化)
    m_pShieldCollider = std::make_shared<SphereCollider>();
    m_pShieldCollider->SetRadius(300.0f); 

    m_homingBullets.clear();
    m_hasPlayedCloseRangeEffect = false;
    m_currentEffectHandle = -1;
    m_hasPlayedCloseRangeEffect = false;
    m_currentEffectHandle = -1;
    m_effectTimer = 0;
    m_shieldEffectHandles.clear();

    // 初期ステートの設定
    ChangeState(std::make_shared<EnemyBossStateWalk>());
}

void EnemyBoss::ChangeAnimation(AnimState newAnimState, bool loop)
{
    // 遠距離攻撃など、同じ状態でも再度再生したいケースがある場合は調整
    if (m_currentAnimState == newAnimState && newAnimState != AnimState::Attack && newAnimState != AnimState::LongRangeAttack) return;

    const char* animName = nullptr;

    // アニメーション切り替え時にエフェクトが残っていたら停止
    if (m_currentEffectHandle != -1)
    {
        StopEffekseer3DEffect(m_currentEffectHandle);
        m_currentEffectHandle = -1;
        m_effectTimer = 0;
    }

    switch (newAnimState)
    {
    case AnimState::Idle:
        animName = EnemyBossConstants::kWalkAnimName;
        break;
    case AnimState::Walk:
        animName = EnemyBossConstants::kWalkAnimName;
        break;
    case AnimState::Attack:
        animName = EnemyBossConstants::kCloseAttackAnimName;
        break;
    case AnimState::LongRangeAttack:
        animName = EnemyBossConstants::kLongRangeAttackAnimName;
        break;
    case AnimState::Dead:
        animName = EnemyBossConstants::kDeadAnimName;
        break;
    }

    if (animName)
    {
        m_animationManager.PlayAnimation(m_modelHandle, animName, loop);
        m_animTime = 0.0f;
    }
    m_currentAnimState = newAnimState;
}

void EnemyBoss::ChangeState(std::shared_ptr<EnemyState<EnemyBoss>> newState)
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

void EnemyBoss::Update(const EnemyUpdateContext& context)
{
    // コンテキストから展開
    std::vector<Bullet>& bullets = context.bullets;
    const Player::TackleInfo& tackleInfo = context.tackleInfo;
    const Player& player = context.player;
    const std::vector<Stage::StageCollisionData>& collisionData = context.collisionData;
    Effect* pEffect = context.pEffect;

    UpdateStageCollision(collisionData, context.collisionGrid);

    // 初回更新時にプレイヤーの方へ強制的に向く
    if (m_isFirstUpdate)
    {
        VECTOR toPlayer = VSub(player.GetPos(), m_pos);
        toPlayer.y = 0.0f;
        if (VSquareSize(toPlayer) > 0.0001f)
        {
            float yaw = atan2f(toPlayer.x, toPlayer.z) + DX_PI_F;
            MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, yaw, 0.0f));
        }
        m_isFirstUpdate = false;
    }

#ifdef _DEBUG
    m_shouldDrawParryCollider = false;
#endif

    // ホーミング弾の更新
    for (auto& bullet : m_homingBullets)
    {
        if (!bullet.active) continue;

        // プレイヤーへの方向
        VECTOR toPlayer = VSub(player.GetPos(), bullet.pos);
        float distToPlayer = VSize(toPlayer);
        VECTOR targetDir = VNorm(toPlayer);

        // パラボリックかホーミングかで分岐
        float scale = Game::GetTimeScale();
        if (bullet.isParabolic)
        {
            // 放物線運動
            bullet.velocity.y -= bullet.gravity * scale;
            bullet.pos = VAdd(bullet.pos, VScale(bullet.velocity, scale));
            // 進行方向を速度ベクトルに合わせる（見た目のため）
            if (VSquareSize(bullet.velocity) > 0.0001f)
            {
                bullet.dir = VNorm(bullet.velocity);
            }
        }
        else
        {
            // ホーミング処理 (現在の向きからターゲット向きへ徐々に補間)
            // 戻ってくる動きを応用 -> プレイヤーが動いても追従
            // シンプルにターンレートで補間
            bullet.dir = VAdd(bullet.dir, VScale(targetDir, EnemyBossConstants::kHomingTurnRate * scale));
            bullet.dir = VNorm(bullet.dir);

            // 移動
            VECTOR moveVec = VScale(bullet.dir, bullet.speed * scale);
            bullet.pos = VAdd(bullet.pos, moveVec);
            bullet.distTraveled += bullet.speed * scale;
        }

        // エフェクト更新(あれば)
        if (bullet.effectHandle != -1)
        {
            SetPosPlayingEffekseer3DEffect(bullet.effectHandle, bullet.pos.x, bullet.pos.y, bullet.pos.z);
        }

        // まだ反射されていない弾の処理
        if (!bullet.isReflected)
        {
            SphereCollider bulletCol(bullet.pos, EnemyBossConstants::kHomingBulletRadius);
            bool hitDetected = false;

            // パリィ判定
            if (bullet.isParryable && player.IsJustGuarded())
            {
                VECTOR playerCapA, playerCapB;
                float playerRadius;
                player.GetCapsuleInfo(playerCapA, playerCapB, playerRadius);

                // パリィしやすくするために判定を広くする
                float parryRadius = playerRadius * 1.5f;
                CapsuleCollider parryCollider(playerCapA, playerCapB, parryRadius);

#ifdef _DEBUG
                m_shouldDrawParryCollider = true;
                m_debugParryCapA = playerCapA;
                m_debugParryCapB = playerCapB;
                m_debugParryRadius = parryRadius;
#endif

                if (bulletCol.IsIntersects(&parryCollider))
                {
                    // パリィ成功
                    hitDetected = true;
                    bullet.isReflected = true;

                    // パリィ成功SEを再生
                    const_cast<Player&>(player).PlayParrySE();

                    // チュートリアルマネージャーに通知
                    TaskTutorialManager::GetInstance()->NotifyParrySuccess();

                    // 反射方向計算 (プレイヤーカメラの前方へ、あるいはボスへ)
                    // ここではボス（発射主）へ跳ね返す
                    if (bullet.owner)
                    {
                        VECTOR targetPos = bullet.owner->GetPos();
                        targetPos.y += EnemyBossConstants::kBodyColliderHeight * 0.5f; // 中心付近へ
                        bullet.dir = VNorm(VSub(targetPos, bullet.pos));
                    }
                    else
                    {
                        bullet.dir = VScale(bullet.dir, -1.0f); // 単純反転
                    }

                    bullet.speed *= 1.5f;
                    bullet.turnRate = 0.0f; // 反射後はホーミング切る
                    Game::SetTimeScale(0.1f, 1.0f);
                }
            }

            if (!hitDetected)
            {
                std::shared_ptr<CapsuleCollider> pCol = player.GetBodyCollider();
                if (bulletCol.IsIntersects(pCol.get()))
                {
                    hitDetected = true;
                    // ダメージ処理
                    const_cast<Player&>(player).TakeDamage(bullet.damage, m_pos, bullet.isParryable);
                    bullet.active = false;
                    if (bullet.effectHandle != -1)
                    {
                        StopEffekseer3DEffect(bullet.effectHandle);
                        bullet.effectHandle = -1;
                    }
                }
            }
        }
        // 反射された弾の処理
        else
        {
            // 弾の所有者(Boss自身)と当たり判定
            if (bullet.owner)
            {
                SphereCollider reflectedCol(bullet.pos, EnemyBossConstants::kHomingBulletRadius);
                // ボスの当たり判定を使用
                if (reflectedCol.IsIntersects(this->GetBodyCollider().get()))
                {
                    this->TakeDamage(bullet.damage, AttackType::Parry);
                    this->OnParried(); // 怯み処理
                    bullet.active = false;
                    if (bullet.effectHandle != -1)
                    {
                        StopEffekseer3DEffect(bullet.effectHandle);
                        bullet.effectHandle = -1;
                    }
                }
            }
        }

        // 最大飛距離チェック
        if (bullet.distTraveled > EnemyBossConstants::kHomingBulletMaxDist)
        {
            bullet.active = false;
            if (bullet.effectHandle != -1)
            {
                StopEffekseer3DEffect(bullet.effectHandle);
                bullet.effectHandle = -1;
            }
        }

        // 地面接触で消滅
        if (bullet.pos.y < 0)
        {
            bullet.active = false;
            if (bullet.effectHandle != -1)
            {
                StopEffekseer3DEffect(bullet.effectHandle);
                bullet.effectHandle = -1;
            }
        }
    }

    // 不要な弾を削除
    m_homingBullets.erase(
        std::remove_if(m_homingBullets.begin(), m_homingBullets.end(), [](const HomingBullet& b) { return !b.active; }),
        m_homingBullets.end());

    // 怯み状態の処理
    if (m_isStunned)
    {
        m_stunTimer--;
        if (m_stunTimer <= 0)
        {
            m_isStunned = false;
            ChangeAnimation(AnimState::Walk, true);
        }
        else
        {
            // アニメーション更新（Deadモーションなどを途中まで再生するなど）
            // 一旦アニメーションを進める
            if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
            {
                m_animTime += 1.0f * Game::GetTimeScale();
                m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
            }
        }
        MV1SetPosition(m_modelHandle, m_pos);
        return; // 他のAIロジックをスキップ
    }

    if (m_hp <= 0.0f)
    {
        for (int handle : m_shieldEffectHandles)
        {
            StopEffekseer3DEffect(handle);
        }
        m_shieldEffectHandles.clear();
        UpdateDeath(collisionData);
        return;
    }

    MV1SetPosition(m_modelHandle, m_pos);

    // プレイヤーの位置・コライダー
    VECTOR playerPos = player.GetPos();
    std::shared_ptr<CapsuleCollider> playerBodyCollider = player.GetBodyCollider();

    // AIステートの更新
    if (m_pCurrentState)
    {
        m_pCurrentState->Update(this, context);
    }

    // 攻撃範囲コライダー更新
    VECTOR attackRangeCenter = m_pos;
    attackRangeCenter.y += (EnemyBossConstants::kBodyColliderHeight * 0.5f);
    m_pAttackRangeCollider->SetCenter(attackRangeCenter);

    // 状態遷移ロジック
    if (m_currentAnimState == AnimState::Attack)
    {
        // 攻撃中
        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyBossConstants::kCloseAttackAnimName);

        // 攻撃アニメーション終了判定
        if (m_animTime > currentAnimTotalTime)
        {
            if (m_attackEndDelayTimer <= 0)
            {
                m_attackEndDelayTimer = EnemyBossConstants::kAttackEndDelay;
            }
        }

        // デバッグ用ヒット表示も更新 (ボスの位置に追従させる)
        // 常に更新しないと、攻撃アニメーション中に移動した場合に表示が置いていかれる
        // 球から縦長のカプセルに変更し、高さズレによる空振りを防ぐ
        VECTOR hitBasePos = m_pos;
        VECTOR hitTopPos = m_pos;
        hitTopPos.y += EnemyBossConstants::kBodyColliderHeight;
        m_pAttackHitCollider->SetSegment(hitBasePos, hitTopPos);
        m_pAttackHitCollider->SetRadius(m_pAttackRangeCollider->GetRadius());

        // 攻撃ヒット判定
        // 判定期間 (0.4 ~ 0.6)
        if (m_animTime >= currentAnimTotalTime * 0.4f && m_animTime <= currentAnimTotalTime * 0.6f)
        {
            if (!m_hasAttackHit)
            {
                // 範囲攻撃判定
                if (m_pAttackHitCollider->IsIntersects(playerBodyCollider.get()))
                {
                    // ダメージを与える
                    const_cast<Player&>(player).TakeDamage(static_cast<float>(m_attackPower), m_pos);
                    m_hasAttackHit = true;
                }
            }
        }

        // エフェクト再生 (ヒット判定開始タイミングに合わせて再生)
        if (!m_hasPlayedCloseRangeEffect && m_animTime > currentAnimTotalTime * 0.3f)
        {
            if (pEffect)
            {
                // 足元を中心に再生
                m_currentEffectHandle = pEffect->PlayCloseRangeAttackEffect(m_pos.x, m_pos.y, m_pos.z);
                m_effectTimer = 100; // 100フレームで強制終了
            }
            m_hasPlayedCloseRangeEffect = true;
        }
        
        // エフェクトタイマー更新
        if (m_currentEffectHandle != -1)
        {
            m_effectTimer--;
            if (m_effectTimer <= 0)
            {
                StopEffekseer3DEffect(m_currentEffectHandle);
                m_currentEffectHandle = -1;
            }
        }

        if (m_attackEndDelayTimer > 0)
        {
            --m_attackEndDelayTimer;
            if (m_attackEndDelayTimer == 0)
            {
                m_hasAttackHit = false;
                // 攻撃終了後、範囲内にいれば再度攻撃、いなければ移動へ
                if (CanAttackPlayer(player))
                {
                    ChangeState(std::make_shared<EnemyBossStateAttack>());
                }
                else
                {
                    ChangeState(std::make_shared<EnemyBossStateWalk>());
                }
            }
        }
    }
    else if (m_currentAnimState == AnimState::LongRangeAttack)
    {
        float totalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyBossConstants::kLongRangeAttackAnimName);

        // 弾生成タイミング
        if (!m_hasShotLongRange && m_animTime > totalTime * 0.3f)
        {
            // 弾生成
            VECTOR spawnPos = m_pos;
            if (m_headTopEndNodeIndex != -1)
            {
                spawnPos = MV1GetFramePosition(m_modelHandle, m_headTopEndNodeIndex);
            }
            else
            {
                // 見つからない場合は頭付近オフセット
                spawnPos = VAdd(m_pos, VGet(0, EnemyBossConstants::kBodyColliderHeight, 0));
            }

            HomingBullet bullet;
            bullet.pos = spawnPos;
            bullet.active = true;
            bullet.damage = EnemyBossConstants::kHomingBulletDamage; // ダメージ20
            bullet.distTraveled = 0.0f;
            // プレイヤー方向へ発射
            VECTOR toTarget = VSub(playerPos, spawnPos);
            bullet.dir = VNorm(toTarget);
            bullet.speed = EnemyBossConstants::kHomingBulletSpeed;
            bullet.owner = this;
            bullet.isReflected = false;

            // 放物線攻撃判定
            std::string groundedObj = player.GetGroundedObjectName();
            if ((groundedObj == "Rock3" || groundedObj == "Rock6") &&
                !EnemyBase::IsTargetVisible(spawnPos, playerPos, collisionData, context.collisionGrid))
            {
                bullet.isParabolic = true;
                bullet.gravity = 0.3f; // 重力設定
                bullet.velocity = EnemyBase::CalculateParabolicVelocity(bullet.pos, playerPos, bullet.gravity, EnemyBossConstants::kHomingBulletSpeed);
            }
            else
            {
                bullet.isParabolic = false;
            }

            // エフェクトがあればここで再生しハンドル保持
            if (pEffect)
            {
                // マズルフラッシュ（射撃時の一瞬のエフェクト）
                pEffect->PlayMuzzleFlash(spawnPos.x, spawnPos.y, spawnPos.z, 0, 0, 0);

                if (m_isNextAttackNormal)
                {
                    // 通常弾 (パリィ不可)
                    bullet.isParryable = false;
                    bullet.effectHandle = pEffect->PlayNormalBulletEffect(spawnPos.x, spawnPos.y, spawnPos.z);
                }
                else
                {
                    // パリィ弾 (パリィ可能)
                    bullet.isParryable = true;
                    // EnemyAcidと同様のエフェクトを使用（または専用エフェクト）
                    bullet.effectHandle = pEffect->PlayAcidEffect(spawnPos.x, spawnPos.y, spawnPos.z);
                }
            }

            // 次回のためにフラグ反転
            m_isNextAttackNormal = !m_isNextAttackNormal;

            m_homingBullets.push_back(bullet);
            m_hasShotLongRange = true;
        }

        if (m_animTime >= totalTime)
        {
            // アニメーション終了
            if (m_attackEndDelayTimer <= 0) m_attackEndDelayTimer = 30; // 硬直
        }

        if (m_attackEndDelayTimer > 0)
        {
            m_attackEndDelayTimer--;
            if (m_attackEndDelayTimer == 0)
            {
                ChangeState(std::make_shared<EnemyBossStateWalk>());
                m_longRangeAttackCooldown = EnemyBossConstants::kLongRangeAttackCooldownMax;
            }
        }
    }
    else if (m_currentAnimState == AnimState::Idle || m_currentAnimState == AnimState::Walk)
    {
        // クールダウン減少
        if (m_longRangeAttackCooldown > 0) m_longRangeAttackCooldown--;

        VECTOR toPlayer = VSub(playerPos, m_pos);
        toPlayer.y = 0.0f;
        float disToPlayer = VSize(toPlayer);

        // 向き変更
        float rotSpeed = 0.05f * Game::GetTimeScale();
        RotateTowards(playerPos, rotSpeed);

        // 1. 近接攻撃判定
        if (CanAttackPlayer(player))
        {
            m_hasAttackHit = false;
            ChangeState(std::make_shared<EnemyBossStateAttack>());
        }
        // 2. 遠距離攻撃判定 (クールダウン中でなく、適切な距離にいる場合)
        else if (disToPlayer > EnemyBossConstants::kLongRangeAttackMinDist && 
                 disToPlayer < EnemyBossConstants::kLongRangeAttackMaxDist && 
                 m_longRangeAttackCooldown <= 0)
        {
            m_hasShotLongRange = false;
            ChangeState(std::make_shared<EnemyBossStateLongRange>());
        }
        // 3. 移動 (どちらの攻撃もできない、または移動が必要な距離にいる場合)
        else
        {
            // プレイヤーに近づく
            VECTOR dir = VNorm(toPlayer);
            m_pos = VAdd(m_pos, VScale(dir, m_chaseSpeed * Game::GetTimeScale()));

            // アニメーションをWalkに
            if (m_currentAnimState != AnimState::Walk)
            {
                ChangeAnimation(AnimState::Walk, true);
            }
        }
    }

    // アニメーション更新
    // ハンドルが取得できなくても時間は進める（フリーズ防止）
    {
        const char* animName = nullptr;
        float animSpeed = 1.0f;

        if (m_currentAnimState == AnimState::Walk)
        {
            animName = EnemyBossConstants::kWalkAnimName;
            animSpeed = 0.9f; // 移動速度に合わせて調整 (0.6 -> 0.9)
        }
        else if (m_currentAnimState == AnimState::Idle)
        {
            animName = EnemyBossConstants::kWalkAnimName;
            animSpeed = 0.0f; // 待機中はアニメーション停止
        }
        else if (m_currentAnimState == AnimState::Attack)
        {
            animName = EnemyBossConstants::kCloseAttackAnimName;
        }
        else if (m_currentAnimState == AnimState::Dead)
        {
            animName = EnemyBossConstants::kDeadAnimName;
        }
        else if (m_currentAnimState == AnimState::LongRangeAttack)
        {
            animName = EnemyBossConstants::kLongRangeAttackAnimName;
        }

        animSpeed *= Game::GetTimeScale();

        if (animName)
        {
            m_animTime += animSpeed;

            float totalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, animName);
            
            // アニメーション総時間が取得できた場合のみループ処理を行う
            if (totalTime > 0.0f)
            {
                // ループ処理
                if (m_currentAnimState == AnimState::Walk /*|| m_currentAnimState == AnimState::Idle*/)
                {
                    m_animTime = fmodf(m_animTime, totalTime);
                }
            }

            // アニメーションがアタッチされている場合のみ実際のモデル時間を更新
            if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
            {
                m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
            }
        }
    }

    // コライダー更新(Body)
    VECTOR bodyCapA = VAdd(m_pos, VGet(0, EnemyBossConstants::kBodyColliderRadius, 0));
    VECTOR bodyCapB = VAdd(m_pos, VGet(0, EnemyBossConstants::kBodyColliderHeight - EnemyBossConstants::kBodyColliderRadius, 0));
    m_pBodyCollider->SetSegment(bodyCapA, bodyCapB);
    m_pBodyCollider->SetRadius(EnemyBossConstants::kBodyColliderRadius);

    // コライダー更新(Head)
    if (m_headTopEndNodeIndex != -1)
    {
        VECTOR headPos = MV1GetFramePosition(m_modelHandle, m_headTopEndNodeIndex);
        m_pHeadCollider->SetCenter(headPos);
        m_pHeadCollider->SetRadius(EnemyBossConstants::kHeadRadius);
    }
    else if (m_headNodeIndex != -1)
    {
        VECTOR headPos = MV1GetFramePosition(m_modelHandle, m_headNodeIndex);
        m_pHeadCollider->SetCenter(headPos);
        m_pHeadCollider->SetRadius(EnemyBossConstants::kHeadRadius);
    }
    else
    {
        // 頭が見つからない場合のフォールバック（体の上の方）
        m_pHeadCollider->SetCenter(VAdd(m_pos, VGet(0, EnemyBossConstants::kBodyColliderHeight, 0)));
        m_pHeadCollider->SetRadius(EnemyBossConstants::kHeadRadius);
    }

    // プレイヤーとの押し出し処理(共通関数使用)
    float minDist = EnemyBossConstants::kBodyColliderRadius + playerBodyCollider->GetRadius();
    ResolvePlayerCollision(playerBodyCollider, minDist, 0.0001f);



    // シールドとの押し出し処理
    if (!m_isShieldBroken && m_pShieldCollider)
    {
        // 衝突判定用に現在のシールド位置を算出 (コライダーの更新は描画処理と同期して実施)
        VECTOR shieldPos = m_pos;
        shieldPos.y += EnemyBossConstants::kBodyColliderHeight * 0.6f; // 胸のあたり
        
        float shieldRadius = m_pShieldCollider->GetRadius();
        
        // プレイヤーの位置を取得 (確実に現在の位置を使うためGameクラスから取得)
        if (Game::m_pPlayer)
        {
            VECTOR playerPos = Game::m_pPlayer->GetPos();
            VECTOR playerCapA, playerCapB;
            float playerRadius;
            Game::m_pPlayer->GetCapsuleInfo(playerCapA, playerCapB, playerRadius);

            // カプセル(Player)と球(Shield)の最近接点を求める
            VECTOR segVec = VSub(playerCapB, playerCapA);
            VECTOR ptToA = VSub(shieldPos, playerCapA);
            float segLenSq = VSquareSize(segVec);
            float t = 0.0f;
            if (segLenSq > 0.0001f)
            {
                t = VDot(ptToA, segVec) / segLenSq;
                t = (std::max)(0.0f, (std::min)(1.0f, t));
            }
            VECTOR closestPointOnPlayer = VAdd(playerCapA, VScale(segVec, t));

            // シールド中心からプレイヤーの最近接点へのベクトル
            VECTOR pushDir = VSub(closestPointOnPlayer, shieldPos);
            float distSq = VSquareSize(pushDir);
            float minDist = shieldRadius + playerRadius;

            // 完全重なり対策
            if (distSq <= 0.0001f)
            {
                // 重なっている場合は、XZ平面で外へ押し出す (上方向など変な方向へ行くのを防ぐためデフォルトは水平)
                pushDir = VSub(playerPos, m_pos);
                pushDir.y = 0.0f;
                if (VSquareSize(pushDir) > 0.0001f)
                {
                    pushDir = VNorm(pushDir);
                }
                else
                {
                    pushDir = VGet(0.0f, 0.0f, -1.0f); // 適当な方向
                }
                distSq = 0.0f;
            }

            if (distSq < minDist * minDist)
            {
                float dist = sqrtf(distSq);
                float pushLen = minDist - dist;
                
                // 押し出しベクトル正規化
                if (dist > 0.0001f)
                {
                    pushDir = VScale(pushDir, 1.0f / dist);
                }

                // 押し出し
                pushLen += 1.0f; // マージン
                VECTOR newPos = VAdd(playerPos, VScale(pushDir, pushLen));
                Game::m_pPlayer->SetPos(newPos);

                // 上方向に押し出された場合(シールドに乗った場合), 重力による振動を防ぐために垂直速度をリセット
                if (pushDir.y > 0.5f)
                {
                    // Playerクラスに追加した垂直速度リセット関数を呼ぶ
                    // Game::m_pPlayerはconstポインタではないのでそのまま呼べるはずだが,
                    // constアクセッサしかない場合はキャストが必要.
                    // ここではGame::m_pPlayerはPlayer*なのでOK
                    Game::m_pPlayer->ResetVerticalVelocity();
                }
            }
        }
    }

    CheckHitAndDamage(bullets, pEffect);

    // タックル判定
    if (tackleInfo.isTackling && m_hp > 0.0f && tackleInfo.tackleId != m_lastTackleId)
    {
        CapsuleCollider tackleCol(tackleInfo.capA, tackleInfo.capB, tackleInfo.radius);
        if (m_pBodyCollider->IsIntersects(&tackleCol))
        {
            TakeTackleDamage(tackleInfo.damage);
            m_lastTackleId = tackleInfo.tackleId;
        }
    }
    else if (!tackleInfo.isTackling)
    {
        m_lastTackleId = -1;
    }

    // 最終的な位置をモデルに反映
    MV1SetPosition(m_modelHandle, m_pos);

    // シールドエフェクト更新 (移動後の最終位置で更新)
    VECTOR shieldPos = m_pos;
    shieldPos.y += EnemyBossConstants::kBodyColliderHeight * 0.6f; // 胸のあたり

    // シールドコライダーの位置更新
    // 描画・エフェクトと同期させるため, フレーム末尾で更新
    if (m_pShieldCollider)
    {
        m_pShieldCollider->SetCenter(shieldPos);
    }

    // シールドエフェクト制御 (シームレスループ & 回転実装)
    if (!m_isShieldBroken)
    {
        // エフェクト生成ロジック
        // フェードイン30F, 総再生240F を想定 -> 210Fで次を生成して重ねる
        const float kEffectDuration = 240.0f;
        const float kFadeInDuration = 30.0f; 
        const float kOverlapSpawnTime = kEffectDuration - kFadeInDuration;

        // タイマー更新
        m_shieldEffectTimer += 1.0f * Game::GetTimeScale();

        // エフェクトがない, または再生時間が重なり開始時間を超えたら新規生成
        if (m_shieldEffectHandles.empty() || m_shieldEffectTimer >= kOverlapSpawnTime)
        {
            if (pEffect)
            {
                int handle = pEffect->PlayBossShieldEffect(shieldPos.x, shieldPos.y, shieldPos.z);
                if (handle != -1)
                {
                    m_shieldEffectHandles.push_back(handle);
                    // 次の生成までの時間を計測するためにタイマーをリセット
                    m_shieldEffectTimer = 0.0f;
                }
            }
        }

        // 回転更新
        m_shieldRotation += 0.3f * Game::GetTimeScale(); // 回転速度調整
        while (m_shieldRotation >= 360.0f) m_shieldRotation -= 360.0f;

        // 有効なエフェクト全てのパラメータを更新
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

            // 位置・回転更新
            SetPosPlayingEffekseer3DEffect(handle, shieldPos.x, shieldPos.y, shieldPos.z);
            
            // シールドの回転はボスの向きに依存せず, 独自に回転させる
            // X, Z回転は0固定 (垂直に保つ)
            SetRotationPlayingEffekseer3DEffect(handle, 0.0f, (m_shieldRotation * DX_PI_F / 180.0f), 0.0f);

            // 色変更ロジック (青 -> 赤)
            if (m_maxShieldHp > 0.0f)
            {
                float ratio = m_shieldHp / m_maxShieldHp;
                if (ratio < 0.0f) ratio = 0.0f;
                if (ratio > 1.0f) ratio = 1.0f;

                int r = static_cast<int>(255.0f * (1.0f - ratio));
                int g = 0;
                int b = static_cast<int>(255.0f * ratio);
                
                SetColorPlayingEffekseer3DEffect(handle, r, g, b, 255);
            }

            ++it;
        }
    }
    else
    {
        // 破壊されているなら全エフェクト停止
        for (int handle : m_shieldEffectHandles)
        {
             StopEffekseer3DEffect(handle);
        }
        m_shieldEffectHandles.clear();
    }
}

void EnemyBoss::Draw()
{
    if (!m_isAlive && m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) == -1) return;

    // 視錐台カリング (描画最適化) (共通関数使用)
    if (!ShouldDraw(EnemyBossConstants::kDrawDistanceSq, EnemyBossConstants::kDrawNearDistanceSq, 0.0f)) return;

    EnemyBase::IncrementDrawCount();
    MV1DrawModel(m_modelHandle);

    if (s_shouldDrawCollision || s_shouldDrawAttackHit || s_shouldDrawShieldCollision)
    {
        DrawCollisionDebug();
    }
}

void EnemyBoss::TakeDamage(float damage, AttackType type)
{
    if (m_isDeadAnimPlaying) return;

    // シールド破壊判定
    if (!m_isShieldBroken)
    {
        // シールドHPが0以下の状態で盾投げを食らったら破壊
        if (m_shieldHp <= 0.0f && type == AttackType::ShieldThrow)
        {
            m_isShieldBroken = true;
            for (int handle : m_shieldEffectHandles)
            {
                StopEffekseer3DEffect(handle);
            }
            m_shieldEffectHandles.clear();
            // 破壊音やエフェクトなどをここで追加可能
            // 例: Game::PlaySound("ShieldBreak");
        }
        else if (type != AttackType::ShieldThrow) // 盾投げ以外はシールドHPを減らす
        {
            m_shieldHp -= damage;
            if (m_shieldHp < 0.0f)
            {
                m_shieldHp = 0.0f;
            }

            // シールドHPが0になった瞬間に, 破壊可能エフェクトを再生
            if (m_shieldHp <= 0.0f && !m_hasPlayedShieldBreakableEffect)
            {
                if (m_pShieldCollider)
                {
                    // シールドの位置で再生
                    SceneMain::Instance()->GetEffect()->PlayShieldBreakEffect(m_pShieldCollider->GetCenter());
                }
                m_hasPlayedShieldBreakableEffect = true;
            }
            // ボス本体にはダメージが入らない
            return;
        }
        else
        {
            // シールドHPが残っている状態での盾投げは無効（あるいは微小ダメージ？）
            // 現状は無効とする
            return;
        }
    }

    EnemyBase::TakeDamage(damage, type);

}

void EnemyBoss::OnDeath()
{
    bool isHeadShot = (m_lastHitPart == HitPart::Head);
    int addScore = ScoreManager::Instance().AddScore(isHeadShot) * 10; // ボスなのでスコア高め
    if (SceneMain::Instance())
    {
        SceneMain::Instance()->AddScorePopup(addScore, isHeadShot, ScoreManager::Instance().GetCombo());
    }
}

void EnemyBoss::TakeTackleDamage(float damage)
{
    if (m_isDeadAnimPlaying) return;

    EnemyBase::TakeTackleDamage(damage);
}

void EnemyBoss::OnParried()
{
    m_isStunned = true;
    m_stunTimer = 120; // 怯み時間
    ChangeState(std::make_shared<EnemyBossStateStunned>());
}

std::shared_ptr<CapsuleCollider> EnemyBoss::GetBodyCollider() const
{
    return m_pBodyCollider;
}

float EnemyBoss::CalcDamage(float bulletDamage, HitPart part) const
{
    // ボスは硬い, あるいは弱点だけ効くなどの調整が可能
    if (part == HitPart::Head)
    {
        return bulletDamage * 1.5f;
    }
    return bulletDamage * 0.8f; // ボディは少し硬い
}

void EnemyBoss::DrawCollisionDebug() const
{
    // どちらかが有効なら描画処理を行う
    if (!s_shouldDrawCollision && !s_shouldDrawAttackHit && !s_shouldDrawShieldCollision) return;

    // 体
    if (s_shouldDrawCollision && m_pBodyCollider)
    {
        DebugUtil::DrawCapsule(m_pBodyCollider->GetSegmentA(), m_pBodyCollider->GetSegmentB(), m_pBodyCollider->GetRadius(), 16, 0xff0000); // 赤
    }

    // 頭
    if (s_shouldDrawCollision && m_pHeadCollider)
    {
        DebugUtil::DrawSphere(m_pHeadCollider->GetCenter(), m_pHeadCollider->GetRadius(), 16, 0x00ff00); // 緑
    }

    // 攻撃範囲
    if (s_shouldDrawAttackHit && m_pAttackRangeCollider)
    {
        DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetRadius(), 32, 0xffaa00);
    }

    // 攻撃判定
    if (s_shouldDrawAttackHit && m_currentAnimState == AnimState::Attack && m_pAttackHitCollider)
    {
        DebugUtil::DrawCapsule(m_pAttackHitCollider->GetSegmentA(), m_pAttackHitCollider->GetSegmentB(), m_pAttackHitCollider->GetRadius(), 16, 0xff00ff); // マゼンタ
    }
        
    // パリィ判定
    if (s_shouldDrawCollision && m_shouldDrawParryCollider)
    {
        DebugUtil::DrawCapsule(m_debugParryCapA, m_debugParryCapB, m_debugParryRadius, 16, 0xffff00); // 黄色 (パリィ)
    }

    // 攻撃ヒット判定（ダメージ発生期間のみ表示）
    if (s_shouldDrawAttackHit && m_currentAnimState == AnimState::Attack)
    {
        float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyBossConstants::kCloseAttackAnimName);
        // ダメージ発生期間 (0.4 ~ 0.6)
        if (m_animTime >= currentAnimTotalTime * 0.4f && m_animTime <= currentAnimTotalTime * 0.6f)
        {
            if (m_pAttackRangeCollider)
            {
                // 赤色で描画
                DebugUtil::DrawSphere(m_pAttackRangeCollider->GetCenter(), m_pAttackRangeCollider->GetRadius(), 32, 0xff0000);
            }
        }
    }

    // シールド（デバッグ表示）
    if (s_shouldDrawShieldCollision && !m_isShieldBroken && m_pShieldCollider)
    {
        // シアン色で描画
        DebugUtil::DrawSphere(m_pShieldCollider->GetCenter(), m_pShieldCollider->GetRadius(), 16, 0x00ffff);
    }
}

// 死亡時の更新処理
void EnemyBoss::UpdateDeath(const std::vector<Stage::StageCollisionData>& stageCollision)
{
    if (!m_isDeadAnimPlaying)
    {
        // スコア加算処理はTakeDamageで行うのでここでは不要
        ChangeState(std::make_shared<EnemyBossStateDead>());
        m_isDeadAnimPlaying = true;
        m_animTime = 0.0f; // アニメーション時間をリセット
        m_isAlive = true;  // 死亡アニメーション中はtrueのまま
    }

    // 死亡アニメーション中もアニメーション時間を更新
    m_animTime += 1.0f * Game::GetTimeScale();
    if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
    {
        m_animationManager.UpdateAnimationTime(m_modelHandle, m_animTime);
    }

    float currentAnimTotalTime = m_animationManager.GetAnimationTotalTime(m_modelHandle, EnemyBossConstants::kDeadAnimName);
    if (m_animTime >= currentAnimTotalTime)
    {
        if (m_animationManager.GetCurrentAttachedAnimHandle(m_modelHandle) != -1)
        {
            MV1DetachAnim(m_modelHandle, 0);
            m_animationManager.ResetAttachedAnimHandle(m_modelHandle);
        }
        if (m_onDeathCallback)
        {
            m_onDeathCallback(m_pos);
            m_onDeathCallback = nullptr; // 一度だけ呼び出す
        }
        m_isAlive = false; // 死亡アニメーション終了時のみfalseにする
    }
}


bool EnemyBoss::CanAttackPlayer(const Player& player)
{
    // 攻撃範囲コライダー内にプレイヤーがいるか
    auto playerCol = player.GetBodyCollider();
    return m_pAttackRangeCollider->IsIntersects(playerCol.get());
}

// ダメージ適用（シールドヒット時はエフェクトを変えるためオーバーライド）
void EnemyBoss::ApplyBulletDamage(Bullet& bullet, HitPart part, float distSq, Effect* pEffect)
{
    // シールドヒット時
    if (part == HitPart::Shield)
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
                }
            }

            pEffect->PlayShieldHitEffect(hitPos, normal);
        }

        // ダメージ計算と適用 (シールドHP減少)
        float damage = CalcDamage(bullet.GetDamage(), part);
        TakeDamage(damage, bullet.GetAttackType());
        
        // デバッグ表示用
        if (s_shouldShowDamage)
        {
            s_debugLastDamage = damage;
            s_debugDamageTimer = EnemyConstants::kDebugDamageDisplayTimer;
            s_debugHitInfo = "(Shield)";
        }

        // 弾を非アクティブ状態へ移行し、貫通による多重判定を防止する
        bullet.Deactivate();
    }
    else
    {
        // それ以外は基底クラス（出血エフェクトあり）
        EnemyBase::ApplyBulletDamage(bullet, part, distSq, pEffect);
    }
}


// どこに当たったのか判定する
EnemyBase::HitPart EnemyBoss::CheckHitPart(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
    if (m_isDeadAnimPlaying) return HitPart::None;

    HitPart part = HitPart::None;
    float minDistSq = FLT_MAX;
    VECTOR hitPos = VGet(0, 0, 0);

    // シールドとの判定（破壊されていなければ最優先）
    if (!m_isShieldBroken && m_pShieldCollider)
    {
        // まず、Rayの始点がシールド内部にあるかチェック（ショットガン等の貫通対策）
        // 始点が内部にある場合、IsIsIntersectsRayは「出口」の距離を返してしまうため、
        // 内部にある体（Body）の方が「近い」と判定されてすり抜けてしまう。
        VECTOR diff = VSub(rayStart, m_pShieldCollider->GetCenter());
        float distSqFromCenter = VSquareSize(diff);
        float radius = m_pShieldCollider->GetRadius();
        
        if (distSqFromCenter <= radius * radius)
        {
            // 内部にいるなら、即座にシールドヒットとみなす
            minDistSq = 0.0f;
            hitPos = rayStart;
            part = HitPart::Shield;
        }
        else
        {
            VECTOR tmpHitPos;
            float tmpDistSq;
            if (m_pShieldCollider->IsIsIntersectsRay(rayStart, rayEnd, tmpHitPos, tmpDistSq))
            {
                if (tmpDistSq < minDistSq)
                {
                    minDistSq = tmpDistSq;
                    hitPos = tmpHitPos;
                    part = HitPart::Shield;
                }
            }
        }
    }

    // 頭との判定
    if (m_pHeadCollider)
    {
        VECTOR tmpHitPos;
        float tmpDistSq;
        if (m_pHeadCollider->IsIsIntersectsRay(rayStart, rayEnd, tmpHitPos, tmpDistSq))
        {
            if (tmpDistSq < minDistSq)
            {
                minDistSq = tmpDistSq;
                hitPos = tmpHitPos;
                part = HitPart::Head;
            }
        }
    }

    // 体との判定
    if (m_pBodyCollider)
    {
        VECTOR tmpHitPos;
        float tmpDistSq;
        if (m_pBodyCollider->IsIsIntersectsRay(rayStart, rayEnd, tmpHitPos, tmpDistSq))
        {
            if (tmpDistSq < minDistSq)
            {
                minDistSq = tmpDistSq;
                hitPos = tmpHitPos;
                part = HitPart::Body;
            }
        }
    }

    outHtPos = hitPos;
    outHtDistSq = minDistSq;
    return part;
}
