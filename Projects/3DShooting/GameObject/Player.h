#pragma once
#include "AttackType.h"
#include "Camera.h"
#include "EffekseerForDXLib.h"
#include "PlayerEffectManager.h"
#include "PlayerMovement.h"
#include "PlayerShieldSystem.h"
#include "PlayerUI.h"
#include "PlayerWeaponManager.h"
#include "Stage.h"
#include "PlayerStatus.h"
#include "PlayerTackleSystem.h"
#include "PlayerLockOnSystem.h"
#include <memory>
#include <string>
#include <vector>

class Camera;
class Effect;
class Bullet;
class EnemyBase;
class EnemyNormal;
class CapsuleCollider;
class DirectionIndicator;
class ShellCasing;
class AnimationManager;

/// <summary>
/// プレイヤークラス
/// </summary>
class Player
{
public:
    /// <summary>
    /// タックル情報構造体
    /// </summary>
    struct TackleInfo
    {
        VECTOR capA = { 0, 0, 0 }; // タックル判定カプセルのA点
        VECTOR capB = { 0, 0, 0 }; // タックル判定カプセルのB点
        float radius = 0.0f;     // タックル判定カプセルの半径
        float damage = 0.0f;     // タックルのダメージ量
        bool isTackling = false; // タックル中かどうか
        int tackleId = 0;        // タックルID
    };

public:
    Player();
    virtual ~Player();

    void Init(bool isTutorial = false);
    void Update(const std::vector<EnemyBase*>& enemyList,
        const std::vector<Stage::StageCollisionData>& collisionData);
    void Draw3D();
    void DrawShield();

    /// <summary>
    /// エフェクトマネージャーを取得
    /// </summary>
    PlayerEffectManager& GetEffectManager() { return m_effectManager; }

    /// <summary>
    /// カメラを取得する
    /// </summary>
    /// <returns>カメラの共有ポインタ</returns>
    const std::shared_ptr<Camera>& GetCamera() const { return m_pCamera; }

    /// <summary>
    /// 弾を撃つ
    /// </summary>
    /// <param name="bullets">弾のベクター</param>
    void Shoot(std::vector<Bullet>& bullets);

    /// <summary>
    /// プレイヤーがダメージを受ける
    /// </summary>
    /// <param name="damage">受けるダメージ量</param>
    /// <param name="attackerPos">攻撃者の位置（オプション）</param>
    /// <param name="isParryable">パリィ可能かどうか（デフォルトtrue）</param>
    void TakeDamage(float damage, const VECTOR& attackerPos = VGet(0, 0, 0),
        bool isParryable = true);

    /// <summary>
    /// プレイヤーの位置を取得する
    /// </summary>
    /// <returns>プレイヤーの位置</returns>
    VECTOR GetPos() const { return m_modelPos; }

    /// <summary>
    /// プレイヤーの位置を設定する
    /// </summary>
    /// <param name="pos">設定する位置</param>
    void SetPos(const VECTOR& pos) { 
        m_pos = pos; 
        m_modelPos = pos;
        m_movement.SetPos(pos);
    }

    /// <summary>
    /// 弾の取得
    /// </summary>
    /// <returns>弾のベクター</returns>
    std::vector<Bullet>& GetBullets();

    /// <summary>
    ///  プレイヤーがショット可能かどうか
    /// </summary>
    /// <returns>ショット可能ならtrue</returns>
    bool HasShot();

    /// <summary>
    /// タックル情報を取得する
    /// </summary>
    /// <returns>タックル情報</returns>
    TackleInfo GetTackleInfo() const;

    /// <summary>
    /// プレイヤーのカプセルコライダー情報を取得する
    /// </summary>
    /// <param name="capA">カプセルのA点</param>
    /// <param name="capB">カプセルのB点</param>
    /// <param name="radius">カプセルの半径</param>
    void GetCapsuleInfo(VECTOR& capA, VECTOR& capB, float& radius) const;

    /// <summary>
    /// 直前にガードが成功したかどうか
    /// </summary>
    /// <returns>ガード成功ならtrue</returns>
    bool IsJustGuarded() const;

    /// <summary>
    /// パリィ成功時のSEを再生する
    /// </summary>
    void PlayParrySE() const;

    /// <summary>
    /// 体力を加算する
    /// </summary>
    /// <param name="value">加算する体力値</param>
    void AddHp(float value);

    /// <summary>
    /// プレイヤーがガード中かどうか
    /// </summary>
    /// <returns>ガード中ならtrue</returns>
    bool IsGuarding() const { return m_shieldSystem.IsGuarding(); }

    /// <summary>
    /// 現在の体力を取得する
    /// </summary>
    /// <returns>現在の体力</returns>
    float GetHealth() const { return m_status.GetHealth(); }

    /// <summary>
    /// HPバーアニメーション用体力値を取得
    /// </summary>
    float GetHealthBarAnim() const { return m_status.GetHealthBarAnim(); }

    /// <summary>
    /// 体力低下の点滅タイマーを取得
    /// </summary>
    float GetLowHealthBlinkTimer() const { return m_status.GetLowHealthBlinkTimer(); }

    /// <summary>
    /// 弾薬テキストのフラッシュタイマーを取得
    /// </summary>
    float GetAmmoTextFlashTimer() const { return m_ammoTextFlashTimer; }

    /// <summary>
    /// ロックオンしている敵を取得
    /// </summary>
    EnemyBase* GetLockedOnEnemy() const { return m_lockOnSystem.GetLockedOnEnemy(); }

    /// <summary>
    /// 盾システムを取得
    /// </summary>
    const PlayerShieldSystem& GetShieldSystem() const { return m_shieldSystem; }

    // 垂直速度リセット (ボスシールド対策)
    void ResetVerticalVelocity() { m_movement.ResetVerticalVelocity(); }

    /// <summary>
    /// 最大体力を取得する
    /// </summary>
    /// <returns>最大体力</returns>
    float GetMaxHealth() const { return m_status.GetMaxHealth(); }

    /// <summary>
    /// AR弾薬回復用関数
    /// </summary>
    /// <param name="value">弾薬数</param>
    void AddARAmmo(int value);

    /// <summary>
    /// 現在の移動速度を取得
    /// </summary>
    /// <returns>移動速度</returns>
    float GetCurrentSpeed() const { return m_movement.GetCurrentSpeed(); }

    /// <summary>
    /// SG弾薬回復用関数
    /// </summary>
    /// <param name="value">弾薬数</param>
    void AddSGAmmo(int value);

    /// <summary>
    /// 現在の武器の弾薬数を取得する
    /// </summary>
    /// <returns>現在の弾薬数</returns>
    int GetCurrentAmmo() const;

    /// <summary>
    /// 現在の武器の最大弾薬数を取得する
    /// </summary>
    /// <returns>最大弾薬数</returns>
    int GetMaxAmmo() const;

    /// <summary>
    /// 弾薬無限モードを設定する
    /// </summary>
    /// <param name="isInfinite">無限にするかどうか</param>
    void SetInfiniteAmmo(bool isInfinite) { m_isInfiniteAmmo = isInfinite; }

    /// <summary>
    /// 弾薬無限モードかどうかを取得する
    /// </summary>
    /// <returns>弾薬無限モードならtrue</returns>
    bool IsInfiniteAmmo() const { return m_isInfiniteAmmo; }

    /// <summary>
    /// プレイヤーのカプセルコライダー取得
    /// </summary>
    std::shared_ptr<CapsuleCollider> GetBodyCollider() const;

    /// <summary>
    /// 接地しているオブジェクト名を取得する
    /// </summary>
    /// <returns>接地しているオブジェクト名</returns>
    std::string GetGroundedObjectName() const;

    /// <summary>
    /// 無敵モードを設定する
    /// </summary>
    /// <param name="isInvincible">無敵にするかどうか</param>
    void SetInvincible(bool isInvincible) { m_status.SetInvincible(isInvincible); }

    /// <summary>
    /// 無敵モードかどうかを取得する
    /// </summary>
    /// <returns>無敵モードならtrue</returns>
    bool IsInvincible() const { return m_status.IsInvincible(); }

    /// <summary>
    /// 飛行モードを設定する
    /// </summary>
    /// <param name="isFlightMode">飛行モードにするかどうか</param>
    void SetFlightMode(bool isFlightMode) { m_isFlightMode = isFlightMode; }

    /// <summary>
    /// 飛行モードかどうかを取得する
    /// </summary>
    /// <returns>飛行モードならtrue</returns>
    bool IsFlightMode() const { return m_isFlightMode; }

    /// <summary>
    /// ロックオン可能な敵がいるかどうかを取得する
    /// </summary>
    /// <returns>ロックオン可能な敵がいるならtrue</returns>
    bool IsTargetAvailable() const { return m_lockOnSystem.IsTargetAvailable(); }

    /// <summary>
    /// 敵に照準が合っているかどうかを取得する
    /// </summary>
    /// <returns>照準が合っているならtrue</returns>
    bool IsAimingAtEnemy() const { return m_lockOnSystem.IsAimingAtEnemy(); }

    /// <summary>
    /// 体力が低下しているかどうかを取得する
    /// </summary>
    /// <returns>体力が低下しているならtrue</returns>
    bool IsLowHealth() const { return m_status.IsLowHealth(); }

    /// <summary>
    /// 攻撃制限を設定する
    /// </summary>
    /// <param name="allowedAttack">許可する攻撃タイプ</param>
    void SetAttackRestrictions(AttackType allowedAttack);

    /// <summary>
    /// 現在許可されている攻撃タイプを取得する
    /// </summary>
    /// <returns>許可されている攻撃タイプ</returns>
    AttackType GetAllowedAttackType() const { return m_allowedAttackType; }

    /// <summary>
    /// プレイヤーが死亡しているかどうか
    /// </summary>
    /// <returns>死亡しているならtrue</returns>
    bool IsDead() const { return m_status.IsDead(); }

    /// <summary>
    /// 方向インジケーターを設定する
    /// </summary>
    /// <param name="directionIndicator">方向インジケーターのポインタ</param>
    void SetDirectionIndicator(DirectionIndicator* directionIndicator) { m_pDirectionIndicator = directionIndicator; }

    /// <summary>
    /// 銃を揺らす
    /// </summary>
    /// <param name="power">揺れの強さ</param>
    /// <param name="duration">揺れの持続時間</param>
    void ShakeGun(float power, float duration);

    /// <summary>
    /// エフェクトを設定する
    /// </summary>
    /// <param name="pEffect">エフェクトのポインタ</param>
    void SetEffect(Effect* pEffect) { m_pEffect = pEffect; }

    /// <summary>
    /// アニメーションマネージャーを設定する
    /// </summary>
    /// <param name="pAnimManager">アニメーションマネージャーのポインタ</param>
    void SetAnimationManager(AnimationManager* pAnimManager) { m_pAnimManager = pAnimManager; }

    /// <summary>
    /// 現在の武器タイプを取得する
    /// </summary>
    /// <returns>現在の武器タイプ</returns>
    WeaponType GetCurrentWeaponType() const;

    /// <summary>
    /// 武器マネージャーを取得する
    /// </summary>
    /// <returns>武器マネージャーの参照</returns>
    const PlayerWeaponManager& GetWeaponManager() const { return m_weaponManager; }

private:
    /// <summary>
    /// 死亡時の更新処理
    /// </summary>
    void DeathUpdate();

    /// <summary>
    /// 銃の位置を取得する
    /// </summary>
    /// <returns>銃の位置</returns>
    VECTOR GetGunPos() const;

    /// <summary>
    /// 銃の回転を取得する
    /// </summary>
    /// <returns>銃の回転</returns>
    VECTOR GetGunRot() const;

    /// <summary>
    /// 薬莢排出口の位置を取得する
    /// </summary>
    /// <returns>薬莢排出口の位置</returns>
    VECTOR GetEjectionPortPos() const;

    /// <summary>
    /// 武器を切り替える
    /// </summary>
    /// <param name="weaponType">切り替える武器の種類</param>
    void SwitchWeapon(WeaponType weaponType);

    /// <summary>
    /// 射撃処理の更新
    /// </summary>
    void UpdateShooting();

    /// <summary>
    /// 武器切り替え処理の更新
    /// </summary>
    /// <param name="keyState">キー状態</param>
    void UpdateWeaponSwitching(const unsigned char* keyState);

    /// <summary>
    /// 敵接近時のダッシュ解除チェック
    /// </summary>
    /// <param name="enemyList">敵リスト</param>
    void CheckEnemyProximity(const std::vector<EnemyBase*>& enemyList);

    /// <summary>
    /// 視線チェック
    /// </summary>
    /// <param name="start">始点</param>
    /// <param name="end">終点</param>
    /// <param name="collisionData">衝突データ</param>
    /// <returns>視線が通っていればtrue</returns>
    bool CheckLineOfSight(const VECTOR& start, const VECTOR& end,
        const std::vector<Stage::StageCollisionData>& collisionData) const;

private:
    // コンポーネント
    PlayerWeaponManager m_weaponManager;
    PlayerMovement m_movement;
    PlayerShieldSystem m_shieldSystem;
    PlayerEffectManager m_effectManager;
    PlayerStatus m_status;
    PlayerTackleSystem m_tackleSystem;
    PlayerLockOnSystem m_lockOnSystem;

    std::vector<ShellCasing> m_shellCasings;
    std::shared_ptr<Camera> m_pCamera; // カメラのポインタ
    std::vector<Bullet> m_bullets;     // 弾の管理
    Effect* m_pEffect;                 // エフェクトのポインタ
    AnimationManager* m_pAnimManager;  // アニメーションマネージャーのポインタ
    DirectionIndicator* m_pDirectionIndicator; // 方向インジケーター

    // 位置・姿勢
    VECTOR m_pos;
    VECTOR m_modelPos;
    VECTOR m_scale;
    VECTOR m_gunSwayOffset;    // 銃の揺れオフセット
    VECTOR m_gunSwayRotOffset; // 銃の回転揺れオフセット

    // タイマー類
    float m_idleSwayTimer;               // 待機時の揺れタイマー
    float m_ammoTextFlashTimer;          // 弾薬テキストのフラッシュタイマー
    float m_startAnimTimer;              // 開始演出アニメーションタイマー
    float m_startAnimDuration;           // 開始演出アニメーション時間
    float m_uiFadeTimer;                 // UIフェードインタイマー
    float m_uiFadeDuration;              // UIフェードイン時間

    // 入力・状態フラグ
    unsigned char m_prevKeyState[256]{}; // 前回のキー入力状態
    bool m_prevIsGuarding = false;       // 前フレームのガード状態
    bool m_hasShot;                      // プレイヤーがショット可能かどうか
    bool m_isInfiniteAmmo;               // 弾薬無限モードかどうか
    bool m_isFlightMode;                 // 飛行モードかどうか
    bool m_shouldIgnoreGuardInput;       // ガード入力を無視するか
    bool m_isTutorial;                   // チュートリアル中か
    bool m_isStartAnimating;             // 開始演出アニメーション中か
    bool m_hasLandedAtStart;             // 開始時に一度でも着地したか
    bool m_isUiFadeStarted;              // UIフェードインが開始されたか
    bool m_isDamageHandledInThisFrame;   // 同一フレーム内でダメージ演出（SE/振動）を処理したか

    // ハンドルIDなど
    int m_arInitAmmo;                    // ARの初期弾薬数
    int m_sgInitAmmo;                    // SGの初期弾薬数
    int m_arMaxAmmo;                     // ARの最大弾薬数
    int m_sgMaxAmmo;                     // SGの最大弾薬数
    int m_concentrationLineEffectHandle; // 集中線エフェクトハンドル

    AttackType m_allowedAttackType = AttackType::None;
};