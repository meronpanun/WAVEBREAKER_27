#pragma once
#include "AttackType.h"
#include "EffekseerForDXLib.h"
#include "Stage.h"
#include <vector>
#include <tuple>
#include <deque>

class Bullet;
class Camera;
class Effect;
class AnimationManager;
class ShellCasing;
class EnemyBase;

/// <summary>
/// 武器の種類列挙型
/// </summary>
enum class WeaponType
{
    AssaultRifle, // アサルトライフル
    Shotgun       // ショットガン
};

/// <summary>
/// プレイヤーの武器管理クラス
/// </summary>
class PlayerWeaponManager
{
public:
    PlayerWeaponManager();
    ~PlayerWeaponManager();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="arInitAmmo">AR初期弾薬</param>
    /// <param name="sgInitAmmo">SG初期弾薬</param>
    /// <param name="arMaxAmmo">AR最大弾薬</param>
    /// <param name="sgMaxAmmo">SG最大弾薬</param>
    /// <param name="bulletPower">AR弾威力</param>
    /// <param name="sgBulletPower">SG弾威力</param>
    /// <param name="arShootRate">AR射撃レート</param>
    void Init(int arInitAmmo, int sgInitAmmo, int arMaxAmmo, int sgMaxAmmo,
        float bulletPower, float sgBulletPower, float arShootRate = 10.0f);

    // コンテキスト構造体
    struct UpdateContext
    {
        float deltaTime;
        const VECTOR& playerPos;
        Camera* pCamera;
        bool isGuarding;
        bool isDead;
        bool isTackling;
        bool isLockingOn;
        bool isSwitchingWeapon;
        AttackType allowedAttackType;
        bool isInfiniteAmmo;
        const std::vector<EnemyBase*>& enemyList;
        const std::vector<Stage::StageCollisionData>& collisionData;
    };

    struct DrawContext
    {
        const VECTOR& playerPos;
        Camera* pCamera;
        const VECTOR& gunSwayOffset;
        const VECTOR& gunShakeOffset;
        const VECTOR& gunSwayRotOffset;
        float guardAnimTimer;
        float guardAnimDuration;
        bool isSwitchingWeapon;
        float weaponSwitchTimer;
        float weaponSwitchDuration;
        WeaponType previousWeaponType;
        bool isTryingToGuard;
        bool isTackling;
        float startAnimOffsetY;
    };

    /// <summary>
    /// 更新処理
    /// </summary>
    /// <param name="context">更新コンテキスト</param>
    void Update(const UpdateContext& context);

    /// <summary>
    /// 3D描画処理
    /// </summary>
    /// <param name="context">描画コンテキスト</param>
    void Draw3D(const DrawContext& context);

    /// <summary>
    /// アサルトライフルの射撃レート取得
    /// </summary>
    /// <returns>アサルトライフルの射撃レート</returns>
    static float GetARShootRate() { return 10.0f; }

    /// <summary>
    /// 武器を切り替える
    /// </summary>
    /// <param name="weaponType">切り替える武器の種類</param>
    void SwitchWeapon(WeaponType weaponType);

    /// <summary>
    /// 現在の武器の種類取得
    /// </summary>
    /// <returns>現在の武器の種類</returns>
    WeaponType GetCurrentWeaponType() const { return m_currentWeaponType; }

    /// <summary>
    /// 射撃処理
    /// </summary>
    /// <param name="bullets">弾の管理配列</param>
    /// <param name="playerPos">プレイヤーの位置ベクトル</param>
    /// <param name="pCamera">カメラポインタ</param>
    /// <param name="pEffect">エフェクトポインタ</param>
    /// <param name="pAnimManager">アニメーションマネージャーポインタ</param>
    /// <param name="shellCasings">薬莢の管理配列</param>
    void Shoot(std::vector<Bullet>& bullets, const VECTOR& playerPos,
        Camera* pCamera, Effect* pEffect, AnimationManager* pAnimManager,
        std::vector<ShellCasing>& shellCasings);

    /// <summary>
    /// 射撃可能かどうか取得
    /// </summary>
    /// <returns>true: 射撃可能, false: 射撃不可</returns>
    bool CanShoot() const;

    /// <summary>
    /// 現在の弾薬数取得
    /// </summary>
    /// <returns>現在の弾薬数</returns>
    int GetCurrentAmmo() const;

    /// <summary>
    /// 最大弾薬数取得
    /// </summary>
    /// <returns>最大弾薬数</returns>
    int GetMaxAmmo() const;

    /// <summary>
    /// アサルトライフルの弾薬を追加
    /// </summary>
    /// <param name="value">追加する弾薬数</param>
    void AddARAmmo(int value);

    /// <summary>
    /// ショットガンの弾薬を追加
    /// </summary>
    /// <param name="value">追加する弾薬数</param>
    void AddSGAmmo(int value);

    /// <summary>
    /// 弾薬を消費
    /// </summary>
    void ConsumeAmmo();

    /// <summary>
    /// 武器切り替え中かどうか取得
    /// </summary>
    /// <returns>true: 武器切り替え中, false: 武器切り替え中ではない</returns>
    bool IsSwitchingWeapon() const { return m_isSwitchingWeapon; }

    /// <summary>
    /// 武器切り替え中かどうか設定
    /// </summary>
    /// <param name="switching">武器切り替え中ならtrue</param>
    void SetSwitchingWeapon(bool switching) { m_isSwitchingWeapon = switching; }

    /// <summary>
    /// 武器切り替えタイマー取得
    /// </summary>
    /// <returns>武器切り替えタイマー値</returns>
    float GetWeaponSwitchTimer() const { return m_weaponSwitchTimer; }

    /// <summary>
    /// 武器切り替えアニメーション時間取得
    /// </summary>
    /// <returns>武器切り替えアニメーション時間</returns>
    float GetWeaponSwitchDuration() const { return m_weaponSwitchDuration; }

    /// <summary>
    /// 前の武器の種類取得
    /// </summary>
    /// <returns>前の武器の種類</returns>
    WeaponType GetPreviousWeaponType() const { return m_previousWeaponType; }

    /// <summary>
    /// 前の武器が弾薬少ない状態だったかどうか取得
    /// </summary>
    /// <returns>true: 弾薬少ない, false: 弾薬十分</returns>
    bool GetPrevWeaponHadLowAmmo() const { return m_prevWeaponHadLowAmmo; }

    /// <summary>
    /// 前の武器が弾薬切れだったかどうか取得
    /// </summary>
    /// <returns>true: 弾薬切れ, false: 弾薬切れじゃない</returns>
    bool GetPrevWeaponHadNoAmmo() const { return m_prevWeaponHadNoAmmo; }

    /// <summary>
    /// 銃のシェイクオフセット取得
    /// </summary>
    /// <returns>銃のシェイクオフセットベクトル</returns>
    VECTOR GetGunShakeOffset() const { return m_gunShakeOffset; }

    /// <summary>
    /// 弾薬少ない警告かどうか取得
    /// </summary>
    /// <returns>弾薬少ない警告ならtrue</returns>
    bool IsLowAmmo() const { return m_isLowAmmo; }

    /// <summary>
    /// 弾薬切れ警告かどうか取得
    /// </summary>
    /// <returns>弾薬切れ警告ならtrue</returns>
    bool IsNoAmmoWarning() const { return m_isShowingNoAmmoWarning; }

    /// <summary>
    /// 弾薬警告の点滅タイマー取得
    /// </summary>
    /// <returns>点滅タイマーの値</returns>
    float GetLowAmmoBlinkTimer() const { return m_lowAmmoBlinkTimer; }

    /// <summary>
    /// 無限弾薬モード設定
    /// </summary>
    /// <param name="infinite">無限弾薬にするならtrue</param>
    void SetInfiniteAmmo(bool infinite) { m_isInfiniteAmmo = infinite; }

    /// <summary>
    /// 無限弾薬かどうか取得
    /// </summary>
    /// <returns>無限弾薬ならtrue</returns>
    bool IsInfiniteAmmo() const { return m_isInfiniteAmmo; }

    /// <summary>
    /// 銃の位置を取得
    /// </summary>
    /// <param name="playerPos">プレイヤーの位置</param>
    /// <param name="pCamera">カメラのポインタ</param>
    /// <returns>銃の位置ベクトル</returns>
    VECTOR GetGunPos(const VECTOR& playerPos, Camera* pCamera) const;

    /// <summary>
    /// 銃の回転を取得
    /// </summary>
    /// <param name="pCamera">カメラのポインタ</param>
    /// <returns>銃の回転ベクトル</returns>
    VECTOR GetGunRot(Camera* pCamera) const;

    /// <summary>
    /// 薬莢排出口の位置を取得
    /// </summary>
    /// <returns>薬莢排出口の位置</returns>
    VECTOR GetEjectionPortPos() const;

    /// <summary>
    /// 銃のシェイクを開始
    /// </summary>
    /// <param name="power">シェイクの強さ</param>
    /// <param name="duration">シェイクの持続時間</param>
    void ShakeGun(float power, float duration);

    /// <summary>
    /// アサルトライフルモデルハンドル取得
    /// </summary>
    /// <returns>アサルトライフルモデルハンドル</returns>
    int GetARHandle() const { return m_arHandle; }

    /// <summary>
    /// ショットガンモデルハンドル取得
    /// </summary>
    /// <returns>ショットガンモデルハンドル</returns>
    int GetSGHandle() const { return m_sgHandle; }

    /// <summary>
    /// ショットガンアニメーションの更新
    /// </summary>
    /// <param name="pAnimManager">アニメーションマネージャーのポインタ</param>
    /// <param name="deltaTime">経過時間</param>
    void UpdateSGAnimation(AnimationManager* pAnimManager, float deltaTime);

    /// <summary>
    /// 武器モデルのスケールを設定
    /// </summary>
    /// <param name="scale">スケールベクトル</param>
    void SetWeaponScale(const VECTOR& scale);

    /// <summary>
    /// 武器モデルの回転を設定
    /// </summary>
    /// <param name="rot">回転ベクトル</param>
    void SetWeaponRotation(const VECTOR& rot);

private:
    /// <summary>
    /// 銃のモデルが壁や敵に接触しているか計算し、引き込み距離を返す
    /// </summary>
    float CalculatePullBackOffset(
        const VECTOR& playerPos, Camera* pCamera,
        const std::vector<EnemyBase*>& enemyList,
        const std::vector<Stage::StageCollisionData>& collisionData) const;

public:
    /// <summary>
    /// 射撃反動（演出用）のスケールを取得 (0.0f ~ 1.0f)
    /// </summary>
    /// <returns>反動スケール</returns>
    float GetRecoilScale() const;

private:
    // 武器モデルハンドル
    int m_arHandle;
    int m_sgHandle;
    int m_ejectionPortFrame;
    int m_currentWeaponIndex;

    // 弾薬関連
    int m_arAmmo;
    int m_sgAmmo;
    int m_arMaxAmmo;
    int m_sgMaxAmmo;
    float m_bulletPower;
    float m_sgBulletPower;

    // 射撃関連
    float m_shootCooldown;
    float m_shootCooldownTimer;
    float m_arShootRate;

    // 武器切り替え
    WeaponType m_currentWeaponType;
    WeaponType m_previousWeaponType;
    std::vector<WeaponType> m_weaponTypes;
    bool m_isSwitchingWeapon;
    float m_weaponSwitchTimer;
    float m_weaponSwitchDuration;
    bool m_prevWeaponHadLowAmmo;
    bool m_prevWeaponHadNoAmmo;

    // 弾薬警告
    bool m_isLowAmmo;
    bool m_isShowingNoAmmoWarning;
    float m_lowAmmoBlinkTimer;

    // 無限弾薬
    bool m_isInfiniteAmmo;

    // 銃のシェイク
    VECTOR m_gunShakeOffset;
    float m_gunShakeTimer;
    float m_gunShakePower;

    // ショットガンアニメーション
    bool m_isSGAnimPlaying;
    float m_sgAnimTime;
    float m_sgPumpTimer;
    std::deque<float> m_arCartridgeQueue;
    std::deque<float> m_sgCartridgeQueue;

    // 引き込みオフセット
    float m_pullBackOffset;
};
