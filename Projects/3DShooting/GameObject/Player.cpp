#include "Player.h"
#include "AnimationManager.h"
#include "Bullet.h"
#include "Camera.h"
#include "CapsuleCollider.h"
#include "Collision.h"
#include "CollisionGrid.h"
#include "DebugUtil.h"
#include "DirectionIndicator.h"
#include "Effect.h"
#include "EffekseerForDXLib.h"
#include "EnemyBase.h"
#include "EnemyNormal.h"
#include "Game.h"
#include "InputManager.h"
#include "TaskTutorialManager.h"
#include "SceneGameOver.h"
#include "SceneMain.h"
#include "SceneManager.h"
#include "ShellCasing.h"
#include "SoundManager.h"
#include "TransformDataLoader.h"
#include "WaveManager.h"
#include "PlayerStatus.h"
#include "PlayerTackleSystem.h"
#include "PlayerLockOnSystem.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>

namespace PlayerConstants
{
    // カプセルコライダーのサイズ
    constexpr float kCapsuleHeight = 100.0f;
    constexpr float kCapsuleRadius = 50.0f;

    // タックル関連
    constexpr int kTackleDuration = 35;        // タックル持続フレーム数
    constexpr float kTackleHitRange = 450.0f;  // タックルの前方有効距離
    constexpr float kTackleHitRadius = 250.0f; // タックルの横幅（半径）
    constexpr float kTackleHitHeight = 100.0f; // タックルの高さ

    // 銃の揺れ関連の定数
    constexpr float kGunSwayAmount = 0.7f;  // 銃モデルの揺れの強さ
    constexpr float kGunSwayDamping = 0.8f; // 銃モデルの揺れの減衰率

    // Update関連
    constexpr float kFrameRate = 60.0f;
    constexpr float kDeltaTime = 1.0f / kFrameRate;
    constexpr float kPlayerColliderYOffset = 60.0f;
    constexpr float kTackleFov = 100.0f;          // タックル中のカメラFOV
    constexpr float kTackleCameraZOffset = 30.0f; // タックル中のカメラZオフセット
    constexpr float kConcentrationLineEffectZOffset = 15.0f; // 集中線エフェクトのZオフセット
    constexpr float kHpBarAnimSpeed = 1.5f;      // HPバーのアニメーション速度
    constexpr int kLowAmmoThreshold = 10;        // 弾薬が少ないと判断する閾値
    constexpr float kLowHealthThreshold = 30.0f; // 体力が少ないと判断する閾値
    constexpr float kWarningBlinkSpeed = 1.5f;   // 警告UIの点滅速度
    constexpr float kLowHealthEffectMaxAlpha = 0.7f; // 体力低下UIの最大アルファ値
    constexpr float kIdleSwaySpeed = 1.5f;           // 揺れの速さ
    constexpr float kIdleSwayAmount = 0.04f;         // 揺れの量
    constexpr float kLockOnAngleCos = 0.966f;        // cos(15度)
    constexpr float kLockOnMaxScreenOffsetY = 100.0f; // 画面中央からの垂直方向の最大オフセット
    constexpr float kTackleStopMargin = 45.0f; // タックル停止判定のマージン

    // 盾UI関連
    constexpr int kShieldImageGaugeSpacing = 10; // 盾UIとクールダウンゲージの間隔
    constexpr int kShieldImageActiveAlpha = 255; // 使用可能な盾UIのアルファ値
    constexpr int kShieldImageCooldownAlpha = 128; // クールダウン中の盾UIのアルファ値
    constexpr int kShieldUIYPosition = 420;
    constexpr int kShieldUIYOffset = 30; // 盾UIのY軸調整オフセット

    // フォント関連
    constexpr int kDefaultFontThickness = 3; // フォントの太さ
    constexpr int kAmmoFont = 32;            // 弾薬フォントサイズ
    constexpr int kHpFont = 20;              // HPフォントサイズ
    constexpr int kWarningFont = 24;         // 警告フォントサイズ
    constexpr char kDefaultFontName[] = "Arial Black";
    constexpr char kWarningFontName[] = "HGPｺﾞｼｯｸE";
    constexpr int kDefaultFontType = DX_FONTTYPE_ANTIALIASING_EDGE_8X8;

    // ダメージエフェクト
    constexpr float kDamageEffectDuration = 30.0f; // ダメージエフェクトの持続時間
    constexpr int kDamageEffectColorR = 255;
    constexpr int kDamageEffectColorG = 0;
    constexpr int kDamageEffectColorB = 0;

    // 回復エフェクト
    constexpr float kHealEffectDuration = 45.0f; // 回復エフェクトの持続時間
    constexpr int kHealEffectColorR = 0;
    constexpr int kHealEffectColorG = 255;
    constexpr int kHealEffectColorB = 0;

    // 弾薬取得エフェクト
    constexpr float kAmmoEffectDuration = 45.0f; // 弾薬取得エフェクトの持続時間
    constexpr int kAmmoEffectColorR = 255;
    constexpr int kAmmoEffectColorG = 128;
    constexpr int kAmmoEffectColorB = 0;

    // カメラシェイク
    constexpr float kTakeDamageShakePower = 5.0f; // 攻撃を受けた時の揺れの強さ
    constexpr int kTakeDamageShakeDuration = 15;  // 攻撃を受けた時の揺れの持続時間
    constexpr float kARShootShakePower = 4.0f;    // ARを撃った時の揺れの強さ
    constexpr float kSGShootShakePower = 32.0f;   // SGを撃った時の揺れの強さ
    constexpr int kShootShakeDuration = 8;        // 撃った時の揺れの持続時間
    constexpr float kShieldBreakGunShakePower = 10.0f; // 盾破壊時の銃の揺れの強さ
    constexpr float kShieldBreakGunShakeDuration = 30.0f; // 盾破壊時の銃の揺れの持続時間

    // HpUI関連
    constexpr int kHpBarWidth = 200;
    constexpr int kHpBarHeight = 24;
    constexpr int kHpBarMargin = 30;
    constexpr int kHealthUiImageSize = 64;
    constexpr int kHealthUiImageBarSpacing = 10;
    constexpr float kMaxHp = 100.0f;
    constexpr int kHpTextOffsetX = 8;
    constexpr int kHpTextOffsetY = 2;

    // 色関連
    constexpr unsigned int kColorWhite = 0xffffff;
    constexpr unsigned int kColorLowAmmo = 0xd3381c;
    constexpr unsigned int kColorTackleGaugeBorder = 0x5050C8;
    constexpr unsigned int kColorTackleGaugeFill = 0x50B4ff;
    constexpr unsigned int kColorHpBarBg = 0x505050;
    constexpr unsigned int kColorHpBarDamage = 0xFFD700;
    constexpr unsigned int kColorHpBarFill = 0xff4040;
    constexpr unsigned int kColorHpBarBorder = 0x000000;

    // タックルヒット時のSE音量 (DxLibの設定上0〜255が範囲、255で最大音量)
    constexpr int kTackleHitVolume = 255;
    constexpr int kLightLandingVolume = 120; // 通常着地（小さめ）
    constexpr int kHeavyLandingVolume = 180; // 重量着地（中くらい）
}

Player::Player()
    : m_modelPos(VGet(0, 0, 0))
    , m_pEffect(nullptr)
    , m_pCamera(std::make_shared<Camera>())
    , m_pos(VGet(0, 0, 0))
    , m_hasShot(false)
    , m_isDamageHandledInThisFrame(false)
    , m_isStartAnimating(false)
    , m_hasLandedAtStart(false)
    , m_startAnimTimer(0.0f)
    , m_startAnimDuration(60.0f)
    , m_uiFadeTimer(0.0f)
    , m_uiFadeDuration(60.0f)
    , m_isUiFadeStarted(false)
    , m_idleSwayTimer(0.0f)
    , m_ammoTextFlashTimer(0.0f)
{
}

Player::~Player()
{
    // 心音SEが再生中なら停止
    SoundManager::GetInstance()->Stop("Player", "Heartbeat");
}

void Player::Init(bool isTutorial)
{
    m_isTutorial = isTutorial;
    // CSVからPlayerのTransform情報を取得
    auto dataList = TransformDataLoader::LoadDataCSV("data/CSV/CharacterTransfromData.csv");
    for (const auto& data : dataList)
    {
        if (data.name == "Player")
        {
            // 武器モデルのスケールと回転を設定
            m_weaponManager.SetWeaponScale(data.scale);
            m_weaponManager.SetWeaponRotation(data.rot);

            // コンポーネントの初期化
            m_weaponManager.Init(data.arInitAmmo, data.sgInitAmmo, data.arInitAmmo, data.sgInitAmmo,
                data.bulletPower, data.sgBulletPower);
            m_movement.Init(data.pos, data.speed, data.runSpeed, data.scale.x);
            m_shieldSystem.Init(data.maxShieldDurability, data.shieldRegenRate);
            m_status.Init(data.hp);
            m_tackleSystem.Init(data.tackleCooldown, data.tackleSpeed, data.tackleDamage);
            break;
        }
    }
    m_pCamera->Init(); // カメラの初期化
    m_allowedAttackType = AttackType::None; // 攻撃制限をリセット
    m_tackleSystem.ResetCooldown();         // クールタイムをリセット
    m_shouldIgnoreGuardInput = false;       // 入力無視をリセット

    // 開始演出のアニメーション初期化
    m_isStartAnimating = false; // 着地するまで待機
    m_hasLandedAtStart = false; 
    m_startAnimTimer = 0.0f;
    m_startAnimDuration = 60.0f; // 1秒間（60フレーム）
    m_uiFadeTimer = 0.0f;
    m_uiFadeDuration = 60.0f;
    m_isUiFadeStarted = false;
}

void Player::Update(const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData)
{
    unsigned char keyState[256];
    GetHitKeyStateAll(reinterpret_cast<char*>(keyState));

    // コンポーネントの更新
    float deltaTime = PlayerConstants::kDeltaTime * Game::GetTimeScale();
    m_status.Update(deltaTime);

    // 開始アニメーションタイマーの更新
    if (!m_hasLandedAtStart && m_movement.IsOnGround())
    {
        m_hasLandedAtStart = true;
        m_isStartAnimating = true;
    }

    if (m_isStartAnimating)
    {
        m_startAnimTimer += 1.0f * Game::GetTimeScale();
        if (m_startAnimTimer >= m_startAnimDuration)
        {
            m_isStartAnimating = false;
            m_startAnimTimer = m_startAnimDuration;
        }
    }

    // UIフェードインの制御
    if (m_hasLandedAtStart && !m_isStartAnimating)
    {
        m_isUiFadeStarted = true;
    }

    if (m_isUiFadeStarted && m_uiFadeTimer < m_uiFadeDuration)
    {
        m_uiFadeTimer += 1.0f * Game::GetTimeScale();
        if (m_uiFadeTimer > m_uiFadeDuration) m_uiFadeTimer = m_uiFadeDuration;
    }

    bool isInputDisabled = m_isStartAnimating || !m_hasLandedAtStart;

    VECTOR playerPos = m_movement.GetPos();
    bool isGuarding = m_shieldSystem.IsGuarding();
    bool isSwitchingWeapon = m_weaponManager.IsSwitchingWeapon();
    bool isTackling = m_tackleSystem.IsTackling();
    bool isDead = m_status.IsDead();

    // タックル中もコライダーを更新（移動処理は内部でスキップされる）  
    m_movement.Update(deltaTime, m_pCamera.get(), isDead, isTackling, m_isFlightMode, collisionData, isInputDisabled);

    // 着地SEの再生
    if (m_movement.JustLanded())
    {
        float impactVel = fabsf(m_movement.GetImpactVelocity());
        if (!m_hasLandedAtStart)
        {
            SoundManager::GetInstance()->Play("Player", "HeavyLanding");
            m_hasLandedAtStart = true;
            m_isStartAnimating = true; // 着地したのでアニメーション開始
        }
        else if (impactVel >= 10.0f)
        {
            SoundManager::GetInstance()->Play("Player", "HeavyLanding");
        }
        else
        {
            SoundManager::GetInstance()->Play("Player", "LightLanding");
        }
    }

    m_modelPos = m_movement.GetPos();
    // 敵接近時のダッシュ解除
    CheckEnemyProximity(enemyList);

    // 位置同期
    if (!isTackling)
    {
        m_modelPos = m_movement.GetPos();
    }

    // 武器マネージャー更新
    PlayerWeaponManager::UpdateContext weaponContext = {
        deltaTime,        m_modelPos,        m_pCamera.get(),
        isGuarding,       isDead,            isTackling,
        m_lockOnSystem.IsLockingOn(),    isSwitchingWeapon, m_allowedAttackType,
        m_isInfiniteAmmo, enemyList,         collisionData
    };
    m_weaponManager.Update(weaponContext);

    // 武器切り替え
    if (!isInputDisabled)
    {
        UpdateWeaponSwitching(keyState);
    }

    // カメラ位置設定
    m_pCamera->SetPlayerPos(m_modelPos);

    // フレームごとのダメージ演出フラグをリセット
    m_isDamageHandledInThisFrame = false;

    // 低体力警告の心音再生制御
    int heartbeatHandle = SoundManager::GetInstance()->GetHandle("Player", "Heartbeat");
    if (m_status.IsLowHealth() && !isDead)
    {
        if (CheckSoundMem(heartbeatHandle) == 0)
        {
            SoundManager::GetInstance()->Play("Player", "Heartbeat", true);
        }
    }
    else
    {
        if (CheckSoundMem(heartbeatHandle) == 1)
        {
            SoundManager::GetInstance()->Stop("Player", "Heartbeat");
        }
    }

    // カメラ更新
    m_pCamera->Update(isInputDisabled);

    // Sway更新
    float yawDelta = m_pCamera->GetYawDelta();
    m_shieldSystem.Update(deltaTime, m_pCamera.get(), m_modelPos, isGuarding,
        isTackling, isSwitchingWeapon,
        m_weaponManager.GetWeaponSwitchTimer(),
        m_weaponManager.GetWeaponSwitchDuration(), yawDelta,
        m_movement.IsMoving());

    // ガード入力処理
    bool shouldGuard = !isInputDisabled && !isDead && !isTackling && InputManager::GetInstance()->IsPressMouseRight() && !m_shouldIgnoreGuardInput && !m_shieldSystem.IsShieldBroken();
    m_shieldSystem.SetGuarding(shouldGuard);
    bool currentIsGuarding = m_shieldSystem.IsGuarding();

    // シールド投擲
    if (!isInputDisabled && (m_allowedAttackType == AttackType::None || m_allowedAttackType == AttackType::ShieldThrow) && !isDead && !isTackling && !currentIsGuarding && !isSwitchingWeapon && keyState[KEY_INPUT_R] && !m_prevKeyState[KEY_INPUT_R])
    {
        if (m_shieldSystem.IsShieldThrown())
        {
            m_shieldSystem.ImmediateReturnShield(m_modelPos);
        }
        else
        {
            m_shieldSystem.ThrowShield(m_pCamera.get(), m_modelPos);
        }
    }
    else if (m_isTutorial && keyState[KEY_INPUT_R] && !m_prevKeyState[KEY_INPUT_R] && m_allowedAttackType != AttackType::ShieldThrow)
    {
         TaskTutorialManager::GetInstance()->NotifyRestrictedAction(AttackType::ShieldThrow);
    }

    m_shieldSystem.UpdateShieldThrow(deltaTime, m_pCamera.get(), m_modelPos, enemyList, collisionData, m_pEffect, currentIsGuarding, m_prevIsGuarding);
    m_prevIsGuarding = currentIsGuarding; // 更新

    // 銃揺れ計算
    m_gunSwayOffset.x -= yawDelta * PlayerConstants::kGunSwayAmount;
    m_gunSwayOffset.x *= PlayerConstants::kGunSwayDamping;
    m_gunSwayRotOffset.y -= yawDelta * PlayerConstants::kGunSwayAmount * 0.5f;
    m_gunSwayRotOffset.y *= PlayerConstants::kGunSwayDamping;

    // 待機揺れ
    m_idleSwayTimer += deltaTime;
    if (!m_movement.IsMoving())
    {
        VECTOR idleSway = VGet(sinf(m_idleSwayTimer * PlayerConstants::kIdleSwaySpeed * 2.0f) * PlayerConstants::kIdleSwayAmount, cosf(m_idleSwayTimer * PlayerConstants::kIdleSwaySpeed) * PlayerConstants::kIdleSwayAmount, 0.0f);
        m_gunSwayOffset = VAdd(m_gunSwayOffset, idleSway);
    }

    if (m_pEffect) m_pEffect->Update();

    m_weaponManager.UpdateSGAnimation(m_pAnimManager, deltaTime);

    // 射撃
    if (!isInputDisabled)
    {
        UpdateShooting();
    }

    // 右クリックガード解除で入力無視解除
    if (!InputManager::GetInstance()->IsPressMouseRight())
    {
        m_shouldIgnoreGuardInput = false;
    }

    // ロックオン更新
    m_lockOnSystem.Update(m_modelPos, m_pCamera.get(), enemyList, collisionData, isGuarding, m_tackleSystem.GetCooldown());

    // ガードエフェクト更新
    m_shieldSystem.UpdateGuardEffect(m_pEffect, m_pCamera.get(), m_modelPos, isSwitchingWeapon);
    m_shieldSystem.UpdateSparkEffect(m_pEffect, m_modelPos, m_pCamera.get());

    // タックル更新 (開始判定と実行中の処理含む)
    bool isTackleRestricted = (m_allowedAttackType != AttackType::None && m_allowedAttackType != AttackType::Tackle);
    
    // タックル制限の通知 (もしロックオン中に左クリックされたら)
    if (m_isTutorial && isTackleRestricted && m_lockOnSystem.IsLockingOn() && InputManager::GetInstance()->IsTriggerMouseLeft() && !m_tackleSystem.IsTackling())
    {
        TaskTutorialManager::GetInstance()->NotifyRestrictedAction(AttackType::Tackle);
    }

    if (!isTackleRestricted || m_tackleSystem.IsTackling())
    {
        m_tackleSystem.Update(deltaTime, m_lockOnSystem.IsLockingOn(), m_lockOnSystem.GetLockedOnEnemy(), m_modelPos, m_movement, m_pCamera.get(), m_pEffect, enemyList, collisionData, this);
    }
    else
    {
        // 制限中もタイマーだけは更新する
        m_tackleSystem.UpdateCooldownOnly(deltaTime);
    }

    // タックルが実行されていない場合のみ、通常の敵更新を行う
    if (!m_tackleSystem.IsTackling())
    {
        TackleInfo tackleInfo = GetTackleInfo(); // 初期値(isTackling=false)
        for (EnemyBase* enemy : enemyList)
        {
            if (!enemy) continue;
            EnemyUpdateContext context = { m_bullets, tackleInfo, *this, enemyList, collisionData, m_pEffect };
            enemy->Update(context);
        }
    }

    // 弾更新
    Bullet::UpdateBullets(m_bullets, m_modelPos, collisionData);

    if (m_pCamera)
    {
        m_pCamera->SetHeadBobbingState(m_movement.IsMoving(), m_movement.WasRunning());
    }

    if (isDead)
    {
        DeathUpdate();
        return;
    }

    // HP不足による死亡判定はPlayerStatus内で自動的に行われます
    
    std::copy(std::begin(keyState), std::end(keyState), std::begin(m_prevKeyState));

    // HPバーアニメーション、体力低下警告はPlayerStatus::Updateで行われます
    m_effectManager.Update(deltaTime, m_status.IsLowHealth(), m_status.GetLowHealthBlinkTimer());

    if (m_ammoTextFlashTimer > 0.0f)
    {
        m_ammoTextFlashTimer -= 1.0f;
    }

    ShellCasing::UpdateShellCasings(m_shellCasings);
}

void Player::Draw3D()
{
    bool isTackling = m_tackleSystem.IsTackling();
    bool isTryingToGuard = !m_status.IsDead() && !isTackling && InputManager::GetInstance()->IsPressMouseRight() && !m_shouldIgnoreGuardInput && !m_shieldSystem.IsShieldBroken();
    bool isSwitchingWeapon = m_weaponManager.IsSwitchingWeapon();

    float startAnimOffsetY = 0.0f;
    if (!m_hasLandedAtStart)
    {
        startAnimOffsetY = 500.0f;
    }
    else if (m_isStartAnimating)
    {
        float progress = m_startAnimTimer / m_startAnimDuration;
        float easeOut = 1.0f - powf(1.0f - progress, 3.0f);
        startAnimOffsetY = (1.0f - easeOut) * 500.0f;
    }

    // カメラのジャンプ・着地揺れを銃の揺れに反映
    VECTOR totalSway = m_gunSwayOffset;

    PlayerWeaponManager::DrawContext weaponDrawContext = {
        m_modelPos,
        m_pCamera.get(),
        totalSway,
        m_weaponManager.GetGunShakeOffset(),
        m_gunSwayRotOffset,
        m_shieldSystem.GetGuardAnimTimer(),
        m_shieldSystem.GetGuardAnimDuration(),
        isSwitchingWeapon,
        m_weaponManager.GetWeaponSwitchTimer(),
        m_weaponManager.GetWeaponSwitchDuration(),
        m_weaponManager.GetPreviousWeaponType(),
        isTryingToGuard,
        isTackling,
        startAnimOffsetY
    };
    // 弾と薬莢の描画 (武器の最前面描画より前に行うことで、ステージ等に正しく隠れるようにする)
    Bullet::DrawBullets(m_bullets);
    ShellCasing::DrawShellCasings(m_shellCasings);

    m_weaponManager.Draw3D(weaponDrawContext);

    // シールドソーの描画（投げられている場合のみ）
    if (m_shieldSystem.IsShieldThrown())
    {
        m_shieldSystem.DrawShieldThrow(m_pCamera.get(), m_modelPos);
    }
}

void Player::DrawShield()
{
    float startAnimOffsetY = 0.0f;
    if (!m_hasLandedAtStart)
    {
        startAnimOffsetY = 500.0f;
    }
    else if (m_isStartAnimating)
    {
        float progress = m_startAnimTimer / m_startAnimDuration;
        float easeOut = 1.0f - powf(1.0f - progress, 3.0f);
        startAnimOffsetY = (1.0f - easeOut) * 500.0f;
    }

    m_shieldSystem.Draw(m_pCamera.get(), m_modelPos, m_tackleSystem.IsTackling(),
        m_weaponManager.IsSwitchingWeapon(),
        m_weaponManager.GetWeaponSwitchTimer(),
        m_weaponManager.GetWeaponSwitchDuration(), startAnimOffsetY);
}

void Player::DeathUpdate()
{
    if (m_pCamera)
    {
        m_pCamera->PlayDeathAnimation(m_status.GetDeathTimer());
    }
}

// ダメージを受ける処理
void Player::TakeDamage(float damage, const VECTOR& attackerPos, bool isParryable)
{
    if (m_status.IsDead()) return;

    if (m_isTutorial) damage *= 0.5f;

    if (m_status.IsInvincible()) return;

    // ダメージを受けたらダッシュ解除
    m_movement.CancelRunMode();

    // 方向インジケーターに攻撃者の位置を通知
    if (m_pDirectionIndicator && (attackerPos.x != 0.0f || attackerPos.z != 0.0f))
    {
        m_pDirectionIndicator->ShowAttackedEnemyDirection(Vec3(attackerPos));
    }

    if (m_shieldSystem.IsGuarding() && !m_shieldSystem.IsShieldBroken()) // ガード中で盾が壊れていなければ
    {

        // カメラシェイクを発生
        if (m_pCamera)
        {
            m_pCamera->Shake(PlayerConstants::kTakeDamageShakePower, PlayerConstants::kTakeDamageShakeDuration);
        }

        // ガード音を再生
        SoundManager::GetInstance()->Play("Player", "Guard");

        // 盾の前方にスパークエフェクトを再生
        if (m_pEffect)
        {
            VECTOR forward = VNorm(VSub(m_pCamera->GetTarget(), m_pCamera->GetPos()));
            VECTOR effectPos = VAdd(m_modelPos, VScale(forward, 80.0f));
        }

        float remainingDamage = m_shieldSystem.TakeDamage(damage, m_pEffect, m_pCamera.get(), m_modelPos);
        if (remainingDamage > 0)
        {
            // 銃を揺らす
            m_weaponManager.ShakeGun(PlayerConstants::kShieldBreakGunShakePower, PlayerConstants::kShieldBreakGunShakeDuration);
            
            // ステータス更新 (チュートリアル中は死なないロジックを考慮)
            float finalDamage = remainingDamage;
            m_status.TakeDamage(finalDamage);
            if (m_isTutorial && m_status.GetHealth() <= 0.0f)
            {
                m_status.SetHealth(1.0f);
                m_status.SetDead(false);
            }

            // ダメージエフェクトを発動
            m_effectManager.TriggerDamageEffect(PlayerConstants::kDamageEffectDuration, PlayerConstants::kDamageEffectColorR, PlayerConstants::kDamageEffectColorG, PlayerConstants::kDamageEffectColorB);
            
            // 被弾演出（SE/振動）をフレーム内で一度だけ実行
            if (!m_isDamageHandledInThisFrame)
            {
                // 衝撃音の再生
                SoundManager::GetInstance()->Play("Player", "Hit");

                // ボイスの再生
                int randomIndex = 1 + GetRand(3);
                SoundManager::GetInstance()->Play("Player", "Hurt" + std::to_string(randomIndex));
                if (m_pCamera)
                {
                    m_pCamera->Shake(PlayerConstants::kTakeDamageShakePower, PlayerConstants::kTakeDamageShakeDuration);
                }
                m_isDamageHandledInThisFrame = true;
            }
        }

        return; // 盾で防いだ場合はここで処理を終了
    }

    // ガードしていない、または盾が壊れている場合は直接ダメージを受ける
    // ステータス更新 (チュートリアル中は死なないロジックを考慮)
    m_status.TakeDamage(damage);
    if (m_isTutorial && m_status.GetHealth() <= 0.0f)
    {
        m_status.SetHealth(1.0f);
        m_status.SetDead(false);
    }

    // ダメージエフェクトを発動
    m_effectManager.TriggerDamageEffect(PlayerConstants::kDamageEffectDuration, PlayerConstants::kDamageEffectColorR, PlayerConstants::kDamageEffectColorG, PlayerConstants::kDamageEffectColorB);
    
    // 被弾演出（SE/振動）をフレーム内で一度だけ実行
    if (!m_isDamageHandledInThisFrame)
    {
        // 衝撃音の再生
        SoundManager::GetInstance()->Play("Player", "Hit");

        // ボイスの再生
        int randomIndex = 1 + GetRand(3);
        SoundManager::GetInstance()->Play("Player", "Hurt" + std::to_string(randomIndex));
        if (m_pCamera)
        {
            m_pCamera->Shake(PlayerConstants::kTakeDamageShakePower, PlayerConstants::kTakeDamageShakeDuration);
        }
        m_isDamageHandledInThisFrame = true;
    }
}

// 弾の取得
std::vector<Bullet>& Player::GetBullets() { return m_bullets; }

// プレイヤーがショット可能かどうか
bool Player::HasShot()
{
    bool shot = m_hasShot;
    m_hasShot = false; // 状態をリセット
    return shot;       // 撃ったかどうかを返す
}

void Player::Shoot(std::vector<Bullet>& bullets)
{
    m_weaponManager.Shoot(bullets, m_modelPos, m_pCamera.get(), m_pEffect, m_pAnimManager, m_shellCasings);
}

// 銃の位置を取得
VECTOR Player::GetGunPos() const
{
    return m_weaponManager.GetGunPos(m_modelPos, m_pCamera.get());
}

// 銃の向きを取得
VECTOR Player::GetGunRot() const
{
    return m_weaponManager.GetGunRot(m_pCamera.get());
}

// 薬莢の排出位置を取得
VECTOR Player::GetEjectionPortPos() const
{
    return m_weaponManager.GetEjectionPortPos();
}

std::shared_ptr<CapsuleCollider> Player::GetBodyCollider() const
{
    return m_movement.GetBodyCollider();
}

std::string Player::GetGroundedObjectName() const
{
    return m_movement.GetGroundedObjectName();
}

// タックル情報を取得
Player::TackleInfo Player::GetTackleInfo() const
{
    TackleInfo info;
    info.isTackling = m_tackleSystem.IsTackling();
    info.tackleId = m_tackleSystem.GetTackleId();
    info.damage = m_tackleSystem.GetDamage();
    
    VECTOR dir = m_tackleSystem.GetDir();
    // プレイヤーの体の中心位置
    VECTOR bodyCenter = m_modelPos;
    constexpr float kAROffsetY = 20.0f; // ARオフセットY
    bodyCenter.y += kAROffsetY;

    // プレイヤーの前面中心（体の中心から前方へkTackleHitRangeだけ進める）
    VECTOR frontCenter = VAdd(bodyCenter, VScale(dir, PlayerConstants::kTackleHitRange));

    info.capA = bodyCenter;
    info.capB = frontCenter;
    info.radius = PlayerConstants::kTackleHitRadius;
    
    return info;
}

// カプセル情報を取得
void Player::GetCapsuleInfo(VECTOR& capA, VECTOR& capB, float& radius) const
{
    // m_movementのコライダーから直接取得
    auto collider = m_movement.GetBodyCollider();
    capA = collider->GetSegmentA();
    capB = collider->GetSegmentB();
    radius = collider->GetRadius();
}

void Player::AddHp(float value)
{
    m_status.AddHp(value);
    // 回復時にエフェクトを発動
    m_effectManager.TriggerHealEffect(PlayerConstants::kHealEffectDuration, PlayerConstants::kHealEffectColorR, PlayerConstants::kHealEffectColorG, PlayerConstants::kHealEffectColorB);
}

void Player::AddARAmmo(int value)
{
    m_weaponManager.AddARAmmo(value);
    // 弾薬取得時にエフェクトを発動
    m_effectManager.TriggerAmmoEffect(PlayerConstants::kAmmoEffectDuration, PlayerConstants::kAmmoEffectColorR, PlayerConstants::kAmmoEffectColorG, PlayerConstants::kAmmoEffectColorB);
    m_ammoTextFlashTimer = 60.0f;
}

void Player::AddSGAmmo(int value)
{
    m_weaponManager.AddSGAmmo(value);
    // 弾薬取得時にエフェクトを発動
    m_effectManager.TriggerAmmoEffect(PlayerConstants::kAmmoEffectDuration, PlayerConstants::kAmmoEffectColorR, PlayerConstants::kAmmoEffectColorG, PlayerConstants::kAmmoEffectColorB);
    m_ammoTextFlashTimer = 60.0f;
}

int Player::GetCurrentAmmo() const 
{
    return m_weaponManager.GetCurrentAmmo(); 
}

int Player::GetMaxAmmo() const 
{
    return m_weaponManager.GetMaxAmmo(); 
}

void Player::SetAttackRestrictions(AttackType allowedAttack)
{
    m_allowedAttackType = allowedAttack;
}

void Player::ShakeGun(float power, float duration)
{
    m_weaponManager.ShakeGun(power, duration);
}

bool Player::IsJustGuarded() const 
{
    return m_shieldSystem.IsJustGuarded(); 
}

void Player::PlayParrySE() const 
{
    SoundManager::GetInstance()->Play("Player", "Parry"); 
}

WeaponType Player::GetCurrentWeaponType() const
{
    return m_weaponManager.GetCurrentWeaponType();
}

// 武器を切り替える
void Player::SwitchWeapon(WeaponType weaponType)
{
    m_weaponManager.SwitchWeapon(weaponType);
}

bool Player::CheckLineOfSight(const VECTOR& start, const VECTOR& end,
    const std::vector<Stage::StageCollisionData>& collisionData) const
{
    for (const auto& col : collisionData)
    {
        HITRESULT_LINE result = HitCheck_Line_Triangle(start, end, col.v1, col.v2, col.v3);
        if (result.HitFlag)
        {
            return false;
        }
    }
    return true;
}

void Player::CheckEnemyProximity(const std::vector<EnemyBase*>& enemyList)
{
    if (m_tackleSystem.IsTackling()) return;

    for (const auto& enemy : enemyList)
    {
        if (!enemy || !enemy->IsAlive()) continue;

        // プレイヤーと敵の距離をチェック
        // カプセル半径の和 + マージン
        constexpr float kEnemyCollisionDist = 100.0f;
        VECTOR diff = VSub(m_movement.GetPos(), enemy->GetPos());
        float distSq = VSize(diff); // VSizeも2乗を返すわけではないので注意。VSizeはsqrtを取る。
        // ここでは距離そのもので比較
        if (distSq < kEnemyCollisionDist)
        {
            m_movement.CancelRunMode();
            break;
        }
    }
}

void Player::UpdateWeaponSwitching(const unsigned char* keyState)
{
    if (m_shieldSystem.IsGuarding()) return;

    if (keyState[KEY_INPUT_1] && !m_prevKeyState[KEY_INPUT_1])
    {
        // 武器切り替え自体は制限していない
        m_weaponManager.SwitchWeapon(WeaponType::AssaultRifle);
    }
    else if (keyState[KEY_INPUT_2] && !m_prevKeyState[KEY_INPUT_2])
    {
        m_weaponManager.SwitchWeapon(WeaponType::Shotgun);
    }

    // マウスホイールで武器切り替え
    int wheelRot = InputManager::GetInstance()->GetMouseWheelRotVol();
    if (wheelRot != 0)
    {
        WeaponType currentWeapon = m_weaponManager.GetCurrentWeaponType();
        WeaponType nextWeapon = (currentWeapon == WeaponType::AssaultRifle) ? WeaponType::Shotgun : WeaponType::AssaultRifle;
        m_weaponManager.SwitchWeapon(nextWeapon);
    }
}

void Player::UpdateShooting()
{
    // シールド投げチュートリアル中は射撃を許可するため、制限条件からShieldThrowを除外する
    bool isShootRestricted = m_allowedAttackType != AttackType::None && m_allowedAttackType != AttackType::Shoot && m_allowedAttackType != AttackType::ShieldThrow;

    bool isDead = m_status.IsDead();
    bool isTackling = m_tackleSystem.IsTackling();
    bool isLockingOn = m_lockOnSystem.IsLockingOn();

    if (isDead || isShootRestricted || isTackling || m_shieldSystem.IsGuarding() || isLockingOn || m_weaponManager.IsSwitchingWeapon())
    {
        // チュートリアル中の射撃制限通知
        if (m_isTutorial && !isDead && !isTackling && !m_shieldSystem.IsGuarding() && !isLockingOn && !m_weaponManager.IsSwitchingWeapon())
        {
             if (InputManager::GetInstance()->IsPressMouseLeft())
             {
                 // 許可されていないのに撃とうとした（シールド投げタスク中は射撃を許可するため通知を除外）
                 if (m_allowedAttackType != AttackType::Shoot && m_allowedAttackType != AttackType::Shotgun && 
                     m_allowedAttackType != AttackType::ShieldThrow && m_allowedAttackType != AttackType::None)
                 {
                      TaskTutorialManager::GetInstance()->NotifyRestrictedAction(AttackType::Shoot);
                 }
             }
        }
        return;
    }

    if (InputManager::GetInstance()->IsPressMouseLeft() && (m_weaponManager.GetCurrentAmmo() > 0 || m_isInfiniteAmmo) && m_weaponManager.CanShoot())
    {
        m_weaponManager.Shoot(m_bullets, m_modelPos, m_pCamera.get(), m_pEffect, m_pAnimManager, m_shellCasings);
        m_weaponManager.ConsumeAmmo();
    }
}