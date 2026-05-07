#include "PlayerWeaponManager.h"
#include "AnimationManager.h"
#include "Bullet.h"
#include "Camera.h"
#include "Collision.h"
#include "Effect.h"
#include "EnemyBase.h"
#include "Game.h"
#include "PlayerMovement.h"
#include "ShellCasing.h"
#include "SoundManager.h"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace PlayerWeaponConstants
{
    // アサルトライフルオフセット (カメラ相対位置)
    constexpr float kAROffsetX = 72.0f;
    constexpr float kAROffsetY = -70.0f;
    constexpr float kAROffsetZ = 40.0f;

    // アサルトライフルマズルフラッシュエフェクトのオフセット
    constexpr float kARMuzzleFlashEffectOffsetX = -20.0f;
    constexpr float kARMuzzleFlashEffectOffsetY = 30.0f;
    constexpr float kARMuzzleFlashEffectOffsetZ = 80.0f;

    // ショットガンオフセット (カメラ相対位置)
    constexpr float kSGOffsetX = 72.0f;
    constexpr float kSGOffsetY = -120.0f;
    constexpr float kSGOffsetZ = 40.0f;

    // ショットガンマズルフラッシュエフェクトのオフセット
    constexpr float kSGMuzzleFlashEffectOffsetX = 30.0f;
    constexpr float kSGMuzzleFlashEffectOffsetY = 60.0f;
    constexpr float kSGMuzzleFlashEffectOffsetZ = 210.0f;

    // 1秒あたりの発射回数
    constexpr float kARShootRate = 10.0f;
    constexpr float kSGShootRate = 1.3f;

    // 武器切り替えアニメーション
    constexpr float kWeaponSwitchDuration = 1.1f;

    // 弾薬が少ないと判断する閾値
    constexpr int kLowAmmoThreshold = 10;

    // カメラシェイク
    constexpr float kARShootShakePower = 4.0f;
    constexpr float kSGShootShakePower = 32.0f;
    constexpr int kShootShakeDuration = 8;

    // ショットガン距離減衰パラメータ
    constexpr float kSGAttenuationStartDist = 50.0f;
    constexpr float kSGAttenuationEndDist = 300.0f;
    constexpr float kSGMinDamageRatio = 0.1f;

    // Update関連
    constexpr float kFrameRate = 60.0f;
    constexpr float kDeltaTime = 1.0f / kFrameRate;

    // 基準周波数
    constexpr int kBaseSoundFrequency = 44100;

    // 薬莢SEの音量
	constexpr int kARCartridgeSEVolume = 120;
	constexpr int kSGCartridgeSEVolume = 120;
}

PlayerWeaponManager::PlayerWeaponManager()
    : m_arHandle(-1)
    , m_sgHandle(-1)
    , m_ejectionPortFrame(-1)
    , m_arAmmo(0)
    , m_sgAmmo(0)
    , m_arMaxAmmo(0)
    , m_sgMaxAmmo(0)
    , m_bulletPower(0.0f)
    , m_sgBulletPower(0.0f)
    , m_shootCooldown(1.0f / PlayerWeaponConstants::kARShootRate)
    , m_shootCooldownTimer(0.0f)
    , m_arShootRate(PlayerWeaponConstants::kARShootRate)
    , m_currentWeaponType(WeaponType::AssaultRifle)
    , m_previousWeaponType(WeaponType::AssaultRifle)
    , m_currentWeaponIndex(0)
    , m_isSwitchingWeapon(false)
    , m_weaponSwitchTimer(0.0f)
    , m_weaponSwitchDuration(PlayerWeaponConstants::kWeaponSwitchDuration)
    , m_prevWeaponHadLowAmmo(false)
    , m_prevWeaponHadNoAmmo(false)
    , m_isLowAmmo(false)
    , m_isShowingNoAmmoWarning(false)
    , m_lowAmmoBlinkTimer(0.0f)
    , m_isInfiniteAmmo(false)
    , m_gunShakeOffset(VGet(0, 0, 0))
    , m_gunShakeTimer(0.0f)
    , m_gunShakePower(0.0f)
    , m_isSGAnimPlaying(false)
    , m_sgAnimTime(0.0f)
    , m_sgPumpTimer(0.0f)
    , m_pullBackOffset(0.0f)
{
    // アサルトライフルモデルの読み込み
    m_arHandle = MV1LoadModel("data/model/AR.mv1");
    assert(m_arHandle != -1);

    // ショットガンモデルの読み込み
    m_sgHandle = MV1LoadModel("data/model/SG.mv1");
    assert(m_sgHandle != -1);

    // 薬莢排出口フレームのインデックスを検索
    m_ejectionPortFrame = MV1SearchFrame(m_arHandle, "AR_M_Ejection_Port");
}

PlayerWeaponManager::~PlayerWeaponManager()
{
    // モデルの解放
    MV1DeleteModel(m_arHandle);
    MV1DeleteModel(m_sgHandle);
}

void PlayerWeaponManager::Init(int arInitAmmo, int sgInitAmmo, int arMaxAmmo,
    int sgMaxAmmo, float bulletPower,
    float sgBulletPower, float arShootRate)
{
    m_arAmmo = arInitAmmo;
    m_sgAmmo = sgInitAmmo;
    m_arMaxAmmo = arMaxAmmo;
    m_sgMaxAmmo = sgMaxAmmo;
    m_bulletPower = bulletPower;
    m_sgBulletPower = sgBulletPower;
    m_arShootRate = arShootRate;

    // 武器リストの初期化
    m_weaponTypes.push_back(WeaponType::AssaultRifle);
    m_weaponTypes.push_back(WeaponType::Shotgun);

    // 初期武器を設定
    SwitchWeapon(m_weaponTypes[m_currentWeaponIndex]);
}

void PlayerWeaponManager::Update(const UpdateContext& context)
{
    float deltaTime = context.deltaTime;
    const VECTOR& playerPos = context.playerPos;
    Camera* pCamera = context.pCamera;
    bool isGuarding = context.isGuarding;
    bool isDead = context.isDead;
    bool isTackling = context.isTackling;
    bool isLockingOn = context.isLockingOn;
    bool isSwitchingWeapon = context.isSwitchingWeapon;
    AttackType allowedAttackType = context.allowedAttackType;
    bool isInfiniteAmmo = context.isInfiniteAmmo;
    const std::vector<EnemyBase*>& enemyList = context.enemyList;
    const std::vector<Stage::StageCollisionData>& collisionData = context.collisionData;

    m_isInfiniteAmmo = isInfiniteAmmo;

    // クールタイムタイマー減算
    if (m_shootCooldownTimer > 0.0f)
    {
        m_shootCooldownTimer -= deltaTime;
        if (m_shootCooldownTimer < 0.0f) m_shootCooldownTimer = 0.0f;
    }
    
    // ポンプアクションタイマーの更新
    if (m_sgPumpTimer > 0.0f)
    {
        m_sgPumpTimer -= deltaTime;
        if (m_sgPumpTimer <= 0.0f)
        {
            m_sgPumpTimer = 0.0f;
            // ポンプアクション音を再生
            SoundManager::GetInstance()->Play("Weapon", "SGPump");
        }
    }

    // アサルトライフル薬莢SEキューの更新
    for (auto it = m_arCartridgeQueue.begin(); it != m_arCartridgeQueue.end(); )
    {
        *it -= deltaTime;
        if (*it <= 0.0f)
        {
            SoundManager::GetInstance()->Play("Weapon", "ARCartridge");
            it = m_arCartridgeQueue.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // ショットガン薬莢SEキューの更新
    for (auto it = m_sgCartridgeQueue.begin(); it != m_sgCartridgeQueue.end(); )
    {
        *it -= deltaTime;
        if (*it <= 0.0f)
        {
            SoundManager::GetInstance()->Play("Weapon", "SGCartridge");
            it = m_sgCartridgeQueue.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // 武器切り替えアニメーションの更新
    if (m_isSwitchingWeapon)
    {
        m_weaponSwitchTimer += deltaTime;
        if (m_weaponSwitchTimer >= m_weaponSwitchDuration)
        {
            m_isSwitchingWeapon = false;
            m_weaponSwitchTimer = 0.0f;
        }
    }

    // 銃のシェイク処理
    if (m_gunShakeTimer > 0.0f)
    {
        m_gunShakeTimer -= 1.0f * Game::GetTimeScale();
        float shake = sinf(m_gunShakeTimer) * m_gunShakePower;
        m_gunShakeOffset.x = ((float)rand() / RAND_MAX - 0.5f) * shake;
        m_gunShakeOffset.y = ((float)rand() / RAND_MAX - 0.5f) * shake;
        if (m_gunShakeTimer <= 0.0f)
        {
            m_gunShakeOffset = VGet(0, 0, 0);
        }
    }

    // 弾薬低下の警告表示処理
    int currentAmmo = GetCurrentAmmo();
    if (currentAmmo == 0 && !m_isInfiniteAmmo)
    {
        m_isLowAmmo = false;
        m_lowAmmoBlinkTimer += deltaTime;
        m_isShowingNoAmmoWarning = true;
    }
    else if (currentAmmo <= PlayerWeaponConstants::kLowAmmoThreshold && !m_isInfiniteAmmo)
    {
        m_isLowAmmo = true;
        m_lowAmmoBlinkTimer += deltaTime;
        m_isShowingNoAmmoWarning = false;
    }
    else
    {
        m_isLowAmmo = false;
        m_lowAmmoBlinkTimer = 0.0f;
        m_isShowingNoAmmoWarning = false;
    }

    // 銃の引き込み判定
    float targetPullBack =
        CalculatePullBackOffset(playerPos, pCamera, enemyList, collisionData);
    // スムーズに補間 (1秒で目標に近づく程度の速さ)
    m_pullBackOffset += (targetPullBack - m_pullBackOffset) * (1.0f - powf(0.1f, deltaTime * 10.0f));

    // 画面外（カメラの後ろ）に消えないようにクランプ。
    // 初期Zオフセットが60.0f程度なので、80.0f程度を上限とする。
    if (m_pullBackOffset > 80.0f) m_pullBackOffset = 80.0f;
}

void PlayerWeaponManager::Draw3D(const DrawContext& context)
{
    const VECTOR& playerPos = context.playerPos;
    Camera* pCamera = context.pCamera;
    const VECTOR& gunSwayOffset = context.gunSwayOffset;
    const VECTOR& gunShakeOffset = context.gunShakeOffset;
    const VECTOR& gunSwayRotOffset = context.gunSwayRotOffset;
    float guardAnimTimer = context.guardAnimTimer;
    float guardAnimDuration = context.guardAnimDuration;
    bool isSwitchingWeapon = context.isSwitchingWeapon;
    float weaponSwitchTimer = context.weaponSwitchTimer;
    float weaponSwitchDuration = context.weaponSwitchDuration;
    WeaponType previousWeaponType = context.previousWeaponType;
    bool isTryingToGuard = context.isTryingToGuard;
    bool isTackling = context.isTackling;
    float startAnimOffsetY = context.startAnimOffsetY;

    // モデルの位置と回転を更新
    MATRIX rotYaw = MGetRotY(pCamera->GetYaw());
    MATRIX rotPitch = MGetRotX(-pCamera->GetPitch());
    MATRIX modelRot = MMult(rotPitch, rotYaw);

    auto GetWeaponTransform = [&](WeaponType type)
    {
        VECTOR offset;
        int handle = -1;
        switch (type)
        {
        case WeaponType::AssaultRifle:
            offset = VGet(PlayerWeaponConstants::kAROffsetX, PlayerWeaponConstants::kAROffsetY, PlayerWeaponConstants::kAROffsetZ);
            handle = m_arHandle;
            break;
        case WeaponType::Shotgun:
            offset = VGet(PlayerWeaponConstants::kSGOffsetX, PlayerWeaponConstants::kSGOffsetY, PlayerWeaponConstants::kSGOffsetZ);
            handle = m_sgHandle;
            break;
        }
        return std::make_tuple(handle, offset);
    };

    // ガード入力中 または タックル中
    // は武器を非表示にする（武器切り替え中でもガード入力があれば非表示）
    if (isTryingToGuard || isTackling)
    {
        MV1SetVisible(m_arHandle, false);
        MV1SetVisible(m_sgHandle, false);
        return; // ガード中、タックル中は武器を描画しない
    }
    else if (m_isSwitchingWeapon)
    {
        // 前の武器を下に隠す
        auto [prevHandle, prevOffset] = GetWeaponTransform(previousWeaponType);
        if (prevHandle != -1)
        {
            float progress = (std::min)(m_weaponSwitchTimer / (m_weaponSwitchDuration * 0.5f), 1.0f);
            float easeOut = 1.0f - powf(1.0f - progress, 3.0f);
            float yOffset = easeOut * 300.0f;

            // 修正: オフセットをカメラ空間（ローカル）のY軸に適用する
            prevOffset.y -= yOffset;
            prevOffset.y -= startAnimOffsetY;

            VECTOR rotModelOffset = VTransform(prevOffset, modelRot);

            // カメラのボビング（揺れ）を含まない基準位置を計算
            // PlayerPos + Rotated(CameraOffset)
            MATRIX camRotYaw = MGetRotY(pCamera->GetYaw());
            VECTOR cameraBasePos = VAdd(playerPos, VTransform(pCamera->GetOffset(), camRotYaw));

            // カメラ位置を基準にする -> プレイヤー位置基準に変更（ボビング解除）
            VECTOR modelPos = VAdd(cameraBasePos, rotModelOffset);

            VECTOR finalPos = VAdd(VAdd(modelPos, gunSwayOffset), gunShakeOffset);

            // 引き込み分を手前にずらす。さらに内側（左）と上（胸元）に寄せる。
            VECTOR camForward = VTransform(VGet(0, 0, 1), modelRot);
            VECTOR camRight = VTransform(VGet(1, 0, 0), modelRot);
            VECTOR camUp = VTransform(VGet(0, 1, 0), modelRot);

            // 現時点での checkDistance 概算値を使用して進行度を計算
            float checkDistance = (previousWeaponType == WeaponType::AssaultRifle) ? 160.0f : 180.0f;
            float pullProgress = (std::min)(1.0f, m_pullBackOffset / checkDistance);

            // 引き込みによる位置補正 (手前に引き、左上に寄せる)
            finalPos = VSub(finalPos, VScale(camForward, m_pullBackOffset));
            finalPos = VSub(finalPos, VScale(camRight, pullProgress * 60.0f)); // 左に寄せる
            finalPos = VAdd(finalPos, VScale(camUp, pullProgress * 20.0f)); // 上に寄せる

            // 最前面に描画するために Z バッファをクリア
            ClearDrawScreenZBuffer();

            MV1SetPosition(prevHandle, finalPos);

            // 回転の補正 (反時計回りにひねる等)
            VECTOR baseRot = VAdd(VGet(pCamera->GetPitch(), pCamera->GetYaw() + DX_PI_F, 0.0f), gunSwayRotOffset);
            baseRot.z += pullProgress * 1.5f; // 反時計回りにひねる (ラジアン)

            MV1SetRotationXYZ(prevHandle, baseRot);
            MV1SetVisible(prevHandle, true);
            MV1DrawModel(prevHandle);
        }

        // 新しい武器を下から出す
        auto [currentHandle, currentOffset] = GetWeaponTransform(m_currentWeaponType);
        if (currentHandle != -1)
        {
            float progress = (std::max)(0.0f, (m_weaponSwitchTimer - (m_weaponSwitchDuration * 0.5f)) / (m_weaponSwitchDuration * 0.5f));
            float easeOut = 1.0f - powf(1.0f - progress, 3.0f);
            float yOffset = (1.0f - easeOut) * 300.0f;

            // 修正: オフセットをカメラ空間（ローカル）のY軸に適用する
            currentOffset.y -= yOffset;
            currentOffset.y -= startAnimOffsetY;

            VECTOR rotModelOffset = VTransform(currentOffset, modelRot);

            // カメラのボビング（揺れ）を含まない基準位置を計算
            MATRIX camRotYaw = MGetRotY(pCamera->GetYaw());
            VECTOR cameraBasePos = VAdd(playerPos, VTransform(pCamera->GetOffset(), camRotYaw));

            // カメラ位置を基準にする -> プレイヤー位置基準に変更
            VECTOR modelPos = VAdd(cameraBasePos, rotModelOffset);

            VECTOR finalPos = VAdd(VAdd(modelPos, gunSwayOffset), gunShakeOffset);

            // 引き込み分を手前にずらす。さらに内側（左）と上（胸元）に寄せる。
            VECTOR camForward = VTransform(VGet(0, 0, 1), modelRot);
            VECTOR camRight = VTransform(VGet(1, 0, 0), modelRot);
            VECTOR camUp = VTransform(VGet(0, 1, 0), modelRot);

            float checkDistance = (m_currentWeaponType == WeaponType::AssaultRifle) ? 160.0f : 180.0f;
            float pullProgress = (std::min)(1.0f, m_pullBackOffset / checkDistance);

            finalPos = VSub(finalPos, VScale(camForward, m_pullBackOffset));
            finalPos = VSub(finalPos, VScale(camRight, pullProgress * 60.0f)); // 左に寄せる
            finalPos = VAdd(finalPos, VScale(camUp, pullProgress * 20.0f)); // 上に寄せる

            // 最前面に描画するために Z バッファをクリア
            ClearDrawScreenZBuffer();

            MV1SetPosition(currentHandle, finalPos);

            // 回転の補正
            VECTOR baseRot = VAdd(VGet(pCamera->GetPitch(), pCamera->GetYaw() + DX_PI_F, 0.0f), gunSwayRotOffset);
            baseRot.z += pullProgress * 1.5f; // 反時計回りにひねる

            MV1SetRotationXYZ(currentHandle, baseRot);
            MV1SetVisible(currentHandle, true);
            MV1DrawModel(currentHandle);
        }
    }
    else
    {
        // 通常時の武器表示
        auto [currentHandle, modelOffset] = GetWeaponTransform(m_currentWeaponType);
        int otherHandle = (m_currentWeaponType == WeaponType::AssaultRifle) ? m_sgHandle : m_arHandle;
        MV1SetVisible(otherHandle, false);

        if (currentHandle != -1)
        {
            MV1SetVisible(currentHandle, true);

            // ガードアニメーションの進行度を計算
            float guardAnimProgress = guardAnimTimer / guardAnimDuration;
            float gunOffsetY = -200.0f * (1.0f - cosf(guardAnimProgress * DX_PI_F * 0.5f));

            // 修正: オフセットをカメラ空間（ローカル）のY軸に適用する
            modelOffset.y += gunOffsetY;
            modelOffset.y -= startAnimOffsetY;

            VECTOR rotModelOffset = VTransform(modelOffset, modelRot);

            // カメラのボビング（揺れ）を含まない基準位置を計算
            MATRIX camRotYaw = MGetRotY(pCamera->GetYaw());
            VECTOR cameraBasePos = VAdd(playerPos, VTransform(pCamera->GetOffset(), camRotYaw));

            // カメラ位置を基準にする -> プレイヤー位置基準に変更
            VECTOR modelPos = VAdd(cameraBasePos, rotModelOffset);

            // モデルの位置を設定
            VECTOR finalPos = VAdd(VAdd(modelPos, gunSwayOffset), gunShakeOffset);

            // 引き込み分を手前にずらす。さらに内側（左）と上（胸元）に寄せる。
            VECTOR camForward = VTransform(VGet(0, 0, 1), modelRot);
            VECTOR camRight = VTransform(VGet(1, 0, 0), modelRot);
            VECTOR camUp = VTransform(VGet(0, 1, 0), modelRot);

            float checkDistance = (m_currentWeaponType == WeaponType::AssaultRifle) ? 220.0f : 240.0f;
            float pullProgress = (std::min)(1.0f, m_pullBackOffset / checkDistance);

            finalPos = VSub(finalPos, VScale(camForward, m_pullBackOffset));
            finalPos = VSub(finalPos, VScale(camRight, pullProgress * 60.0f)); // 左に寄せる
            finalPos = VAdd(finalPos, VScale(camUp, pullProgress * 20.0f)); // 上に寄せる

            // 最前面に描画するために Z バッファをクリア
            ClearDrawScreenZBuffer();

            MV1SetPosition(currentHandle, finalPos);

            // モデルの回転を設定 (ひねりを加える)
            VECTOR baseRot = VAdd(VGet(pCamera->GetPitch(), pCamera->GetYaw() + DX_PI_F, 0.0f), gunSwayRotOffset);
            baseRot.z += pullProgress * 1.5f; // 反時計回りにひねる

            MV1SetRotationXYZ(currentHandle, baseRot);

            // モデルを描画
            MV1DrawModel(currentHandle);
        }
    }
}

void PlayerWeaponManager::SwitchWeapon(WeaponType weaponType)
{
    // 同じ武器への切り替え、または切り替え中は処理しない
    if (weaponType == m_currentWeaponType || m_isSwitchingWeapon) return;

    // 切り替え前の武器の弾薬警告状態を保存
    const int prevAmmo = GetCurrentAmmo();
    m_prevWeaponHadLowAmmo = (prevAmmo > 0 && prevAmmo <= PlayerWeaponConstants::kLowAmmoThreshold) && !m_isInfiniteAmmo;
    m_prevWeaponHadNoAmmo = (prevAmmo == 0) && !m_isInfiniteAmmo;

    m_isSwitchingWeapon = true;
    m_weaponSwitchTimer = 0.0f;
    m_previousWeaponType = m_currentWeaponType;
    m_currentWeaponType = weaponType;

    // 武器の種類に応じてクールダウンを設定
    switch (m_currentWeaponType)
    {
    case WeaponType::AssaultRifle:
        m_shootCooldown = 1.0f / m_arShootRate;
        break;
    case WeaponType::Shotgun:
        m_shootCooldown = 1.0f / PlayerWeaponConstants::kSGShootRate;
        break;
    default:
        break;
    }
}

void PlayerWeaponManager::Shoot(std::vector<Bullet>& bullets,
    const VECTOR& playerPos, Camera* pCamera,
    Effect* pEffect, AnimationManager* pAnimManager,
    std::vector<ShellCasing>& shellCasings)
{
    // 画面中央（カメラ中心）からレティクル方向へ発射
    VECTOR cameraPos = pCamera->GetPos();
    VECTOR cameraForward = VNorm(VSub(pCamera->GetTarget(), pCamera->GetPos()));

    VECTOR gunPos = GetGunPos(playerPos, pCamera);
    VECTOR gunDir = GetGunRot(pCamera);

    // 画面中央から出ているように見せるため、カメラ前方に小さくオフセット
    VECTOR spawnPos = VAdd(cameraPos, VScale(cameraForward, 0.0f));

    std::string currentShotSECategory = "Weapon";
    std::string currentShotSEName = "";
    int currentModelHandle = -1;
    VECTOR currentMuzzleFlashOffset = VGet(0, 0, 0);

    float shakePower = 0.0f;
    switch (m_currentWeaponType)
    {
    case WeaponType::AssaultRifle:
        bullets.emplace_back(spawnPos, cameraForward, AttackType::Shoot,
            m_bulletPower);
        currentShotSEName = "GunShot";
        currentModelHandle = m_arHandle;
        currentMuzzleFlashOffset = VGet(PlayerWeaponConstants::kARMuzzleFlashEffectOffsetX, PlayerWeaponConstants::kARMuzzleFlashEffectOffsetY, PlayerWeaponConstants::kARMuzzleFlashEffectOffsetZ);
        shakePower = PlayerWeaponConstants::kARShootShakePower;
        break;
    case WeaponType::Shotgun:
        currentShotSEName = "SGShot";
        currentModelHandle = m_sgHandle;
        currentMuzzleFlashOffset = VGet(PlayerWeaponConstants::kSGMuzzleFlashEffectOffsetX, PlayerWeaponConstants::kSGMuzzleFlashEffectOffsetY, PlayerWeaponConstants::kSGMuzzleFlashEffectOffsetZ);
        shakePower = PlayerWeaponConstants::kSGShootShakePower;
        if (pAnimManager)
        {
            pAnimManager->PlayAnimation(m_sgHandle, "Armature.001|Armature.001|lever action_FIRE|Baked frames", false);
            m_isSGAnimPlaying = true;
            m_sgAnimTime = 0.0f;
            m_sgPumpTimer = 0.4f;      // 0.4秒後にポンプアクション音を再生
            m_sgCartridgeQueue.push_back(1.2f); // 1.2秒後に薬莢落下音を再生
        }
        // ショットガンは複数弾をばらけさせて発射
        for (int i = 0; i < 5; ++i)
        {
            float spreadX = ((float)GetRand(100) / 100.0f - 0.5f) * 0.1f;
            float spreadY = ((float)GetRand(100) / 100.0f - 0.5f) * 0.1f;

            VECTOR spreadDir = VAdd(cameraForward, VGet(spreadX, spreadY, 0));
            spreadDir = VNorm(spreadDir);

            bullets.emplace_back(spawnPos, spreadDir, AttackType::Shotgun,
                m_sgBulletPower, PlayerWeaponConstants::kSGAttenuationStartDist,
                PlayerWeaponConstants::kSGAttenuationEndDist, PlayerWeaponConstants::kSGMinDamageRatio);
        }
        break;
    default:
        break;
    }

    // 薬莢SEを予約
    if (m_currentWeaponType == WeaponType::AssaultRifle)
    {
        m_arCartridgeQueue.push_back(0.8f); // 0.8秒後に薬莢落下音を再生

        // 薬莢モデルの生成
        VECTOR ejectionPos = GetEjectionPortPos();
        VECTOR ejectionDir = VGet(sinf(pCamera->GetYaw() + DX_PI_F * 0.5f), 0.5f, cosf(pCamera->GetYaw() + DX_PI_F * 0.5f));
        shellCasings.emplace_back(ejectionPos, ejectionDir);
    }

    // SEを再生
    int freq = static_cast<int>(PlayerWeaponConstants::kBaseSoundFrequency * Game::GetTimeScale());
    if (freq < 100) freq = 100;
    
    int handle = SoundManager::GetInstance()->GetHandle(currentShotSECategory, currentShotSEName);
    if (handle != -1)
    {
        SetFrequencySoundMem(freq, handle);
        SoundManager::GetInstance()->Play(currentShotSECategory, currentShotSEName);
    }

    float rotX = -pCamera->GetPitch();
    float rotY = pCamera->GetYaw();
    float rotZ = 0.0f;

    if (pEffect)
    {
        // 引き込みによる「ひねり」を Z 回転に反映
        float checkDistance = (m_currentWeaponType == WeaponType::AssaultRifle) ? 160.0f : 180.0f;
        float pullProgress = (std::min)(1.0f, m_pullBackOffset / checkDistance);
        rotZ += pullProgress * 1.5f;

        pEffect->PlayMuzzleFlash(gunPos.x, gunPos.y, gunPos.z, rotX, rotY, rotZ);
    }

    // カメラシェイクを発生
    if (pCamera)
    {
        pCamera->Shake(shakePower, PlayerWeaponConstants::kShootShakeDuration);
    }

    // 銃のシェイクを発生
    ShakeGun(shakePower, PlayerWeaponConstants::kShootShakeDuration);

    // クールタイムリセット
    m_shootCooldownTimer = m_shootCooldown;
}

bool PlayerWeaponManager::CanShoot() const
{
    return m_shootCooldownTimer <= 0.0f;
}

int PlayerWeaponManager::GetCurrentAmmo() const
{
    switch (m_currentWeaponType)
    {
    case WeaponType::AssaultRifle:
        return m_arAmmo;
    case WeaponType::Shotgun:
        return m_sgAmmo;
    default:
        return 0;
    }
}

int PlayerWeaponManager::GetMaxAmmo() const
{
    switch (m_currentWeaponType)
    {
    case WeaponType::AssaultRifle:
        return m_arMaxAmmo;
    case WeaponType::Shotgun:
        return m_sgMaxAmmo;
    default:
        return 0;
    }
}

void PlayerWeaponManager::AddARAmmo(int value)
{
    m_arAmmo += value;
    if (m_arAmmo > m_arMaxAmmo) m_arAmmo = m_arMaxAmmo;
}

void PlayerWeaponManager::AddSGAmmo(int value)
{
    m_sgAmmo += value;
    if (m_sgAmmo > m_sgMaxAmmo) m_sgAmmo = m_sgMaxAmmo;
}

void PlayerWeaponManager::ConsumeAmmo()
{
    if (!m_isInfiniteAmmo)
    {
        if (m_currentWeaponType == WeaponType::AssaultRifle)
        {
            m_arAmmo--;
            if (m_arAmmo <= 0 && m_sgAmmo > 0)
            {
                SwitchWeapon(WeaponType::Shotgun);
            }
        }
        else if (m_currentWeaponType == WeaponType::Shotgun)
        {
            m_sgAmmo--;
            if (m_sgAmmo <= 0 && m_arAmmo > 0)
            {
                SwitchWeapon(WeaponType::AssaultRifle);
            }
        }
    }
}

VECTOR PlayerWeaponManager::GetGunPos(const VECTOR& playerPos,
    Camera* pCamera) const
{
    VECTOR modelOffset;
    VECTOR muzzleFlashOffset;

    switch (m_currentWeaponType)
    {
    case WeaponType::AssaultRifle:
        modelOffset = VGet(PlayerWeaponConstants::kAROffsetX, PlayerWeaponConstants::kAROffsetY, PlayerWeaponConstants::kAROffsetZ);
        muzzleFlashOffset = VGet(PlayerWeaponConstants::kARMuzzleFlashEffectOffsetX, PlayerWeaponConstants::kARMuzzleFlashEffectOffsetY, PlayerWeaponConstants::kARMuzzleFlashEffectOffsetZ);
        break;
    case WeaponType::Shotgun:
        modelOffset = VGet(PlayerWeaponConstants::kSGOffsetX, PlayerWeaponConstants::kSGOffsetY, PlayerWeaponConstants::kSGOffsetZ);
        muzzleFlashOffset = VGet(PlayerWeaponConstants::kSGMuzzleFlashEffectOffsetX, PlayerWeaponConstants::kSGMuzzleFlashEffectOffsetY, PlayerWeaponConstants::kSGMuzzleFlashEffectOffsetZ);
        break;
    default:
        modelOffset = VGet(0, 0, 0);
        muzzleFlashOffset = VGet(0, 0, 0);
        break;
    }

    // モデルのオフセットと回転を計算
    MATRIX rotYaw = MGetRotY(pCamera->GetYaw());
    MATRIX rotPitch = MGetRotX(-pCamera->GetPitch());
    MATRIX modelRot = MMult(rotPitch, rotYaw);
    VECTOR rotatedModelOffset = VTransform(modelOffset, modelRot);

    // カメラのボビング（揺れ）を含まない基準位置を計算
    VECTOR cameraBasePos = VAdd(playerPos, VTransform(pCamera->GetOffset(), rotYaw));

    // カメラ基準にする
    VECTOR gunBasePosition = VAdd(cameraBasePos, rotatedModelOffset);

    VECTOR rotatedMuzzleFlashOffset = VTransform(muzzleFlashOffset, modelRot);

    // 基本の銃口位置
    VECTOR gunMuzzlePos = VAdd(gunBasePosition, rotatedMuzzleFlashOffset);

    // 引き込み分を手前にずらす。さらに内側（左）と上（胸元）に寄せる。
    VECTOR camForward = VTransform(VGet(0, 0, 1), modelRot);
    VECTOR camRight = VTransform(VGet(1, 0, 0), modelRot);
    VECTOR camUp = VTransform(VGet(0, 1, 0), modelRot);

    float checkDistance = (m_currentWeaponType == WeaponType::AssaultRifle) ? 160.0f : 180.0f;
    float pullProgress = (std::min)(1.0f, m_pullBackOffset / checkDistance);

    gunMuzzlePos = VSub(gunMuzzlePos, VScale(camForward, m_pullBackOffset));
    gunMuzzlePos = VSub(gunMuzzlePos, VScale(camRight, pullProgress * 60.0f)); // 左に寄せる
    gunMuzzlePos = VAdd(gunMuzzlePos, VScale(camUp, pullProgress * 20.0f)); // 上に寄せる

    return gunMuzzlePos;
}

VECTOR PlayerWeaponManager::GetGunRot(Camera* pCamera) const
{
    return VGet(cosf(pCamera->GetPitch()) * sinf(pCamera->GetYaw()),
        sinf(pCamera->GetPitch()),
        cosf(pCamera->GetPitch()) * cosf(pCamera->GetYaw()));
}

VECTOR PlayerWeaponManager::GetEjectionPortPos() const
{
    int ejectionPortFrame = MV1SearchFrame(m_arHandle, "AR_M_Ejection_Port");

    if (ejectionPortFrame != -1)
    {
        return MV1GetFramePosition(m_arHandle, ejectionPortFrame);
    }
    return VGet(0, 0, 0);
}

void PlayerWeaponManager::ShakeGun(float power, float duration)
{
    m_gunShakePower = power;
    m_gunShakeTimer = duration;
}

void PlayerWeaponManager::SetWeaponScale(const VECTOR& scale)
{
    MV1SetScale(m_arHandle, scale);
    MV1SetScale(m_sgHandle, scale);
}

void PlayerWeaponManager::SetWeaponRotation(const VECTOR& rot)
{
    MV1SetRotationXYZ(m_arHandle, rot);
    MV1SetRotationXYZ(m_sgHandle, rot);
}

void PlayerWeaponManager::UpdateSGAnimation(AnimationManager* pAnimManager,
    float deltaTime)
{
    if (m_isSGAnimPlaying)
    {
        m_sgAnimTime += 1.0f * Game::GetTimeScale();
        if (pAnimManager)
        {
            pAnimManager->UpdateAnimationTime(m_sgHandle, m_sgAnimTime);

            // アニメーションが終了したかチェック
            float totalTime = pAnimManager->GetAnimationTotalTime(m_sgHandle, "Armature.001|Armature.001|lever action_FIRE|Baked frames");
            if (totalTime > 0 && m_sgAnimTime >= totalTime)
            {
                m_isSGAnimPlaying = false;
                int attachIndex = pAnimManager->GetCurrentAttachedAnimHandle(m_sgHandle);
                if (attachIndex != -1)
                {
                    MV1DetachAnim(m_sgHandle, attachIndex);
                    pAnimManager->ResetAttachedAnimHandle(m_sgHandle);
                }
            }
        }
    }
}

float PlayerWeaponManager::CalculatePullBackOffset(const VECTOR& playerPos, Camera* pCamera, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData) const
{
    // カメラが存在しない、または下向き（約-45度以下）の場合は引き込み処理を行わない
    if (!pCamera || pCamera->GetPitch() < -DX_PI_F * 0.25f) return 0.0f;

    // 現在装備中の武器の銃身長をレイの長さとして定義
    float checkDistance = (m_currentWeaponType == WeaponType::AssaultRifle) ? 160.0f : 180.0f;

    VECTOR camPos     = pCamera->GetPos();
    VECTOR camForward = VNorm(VSub(pCamera->GetTarget(), camPos));

    // カメラの少し前方を起点としたレイを形成（自キャラの背中などへの誤判定を防ぐ）
    VECTOR rayStart = VAdd(camPos, VScale(camForward, 10.0f));
    VECTOR rayEnd   = VAdd(rayStart, VScale(camForward, checkDistance));

    float minHitT = 1.0f; // レイの割合(0.0 ～ 1.0)における最短ヒット距離を保持
    bool  isHit   = false;

    // ステージ（静的ポリゴン）とのレイキャスト判定
    for (const auto& poly : collisionData)
    {
        float t = 0.0f;
        // Rayとステージを構成する三角形ポリゴンとの交差判定を実行
        if (Collision::IntersectRayTriangle(rayStart, VScale(camForward, checkDistance), poly.v1, poly.v2, poly.v3, t))
        {
            if (t >= 0.0f && t < minHitT)
            {
                minHitT = t;
                isHit = true;
            }
        }
    }

    // 敵（動的オブジェクト）とのレイキャスト判定
    for (const auto& enemy : enemyList)
    {
        if (!enemy || !enemy->IsAlive()) continue;

        VECTOR hitPos;
        float hitDistSq;
        
        // 敵の各部位に対するRayの当たり判定を実行し、ヒット部位の距離を取得
        if (enemy->CheckHitPart(rayStart, rayEnd, hitPos, hitDistSq) != EnemyBase::HitPart::None)
        {
            // レイの始点からの距離を0.0 ～ 1.0の割合(t)に変換し、最短かをチェック
            float t = sqrtf(hitDistSq) / checkDistance;
            if (t < minHitT)
            {
                minHitT = t;
                isHit = true;
            }
        }
    }

    // 衝突結果を元に銃のダイナミックな引き込み量を計算
    if (isHit)
    {
        // 障害物との距離が近い（tが小さい）ほど、手前への引き込み量を大きく設定
        // ※最前面描画との併用のため、位置の引き下げ自体は控えめにし「銃のひねり演出」を補完する
        return (1.0f - minHitT) * checkDistance * 0.5f;
    }

    return 0.0f;
}

float PlayerWeaponManager::GetRecoilScale() const
{
    if (m_shootCooldown <= 0.0f) return 0.0f;

    // クールダウンの残り時間を正規化して返す
    float progress = m_shootCooldownTimer / m_shootCooldown;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    return progress;
}
