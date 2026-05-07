#pragma once
#include "EffekseerForDXLib.h"
#include "Stage.h"
#include <vector>

class Camera;
class Effect;
class EnemyBase;

/// <summary>
/// プレイヤーのガード・盾管理クラス
/// </summary>
class PlayerShieldSystem
{
public:
    PlayerShieldSystem();
    ~PlayerShieldSystem();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="maxDurability">最大耐久値</param>
    /// <param name="regenRate">回復速度</param>
    void Init(float maxDurability, float regenRate);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update(float deltaTime, Camera* pCamera, const VECTOR& playerPos, bool isGuarding, bool isTackling, bool isSwitchingWeapon, float weaponSwitchTimer, float weaponSwitchDuration, float yawDelta, bool isMoving);

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw(Camera* pCamera, const VECTOR& playerPos, bool isTackling, bool isSwitchingWeapon, float weaponSwitchTimer, float weaponSwitchDuration, float startAnimOffsetY = 0.0f);

    /// <summary>
    /// ガード状態かどうか
    /// </summary>
    /// <returns>true: ガード状態, false: ガード状態でない</returns>
    bool IsGuarding() const { return m_isGuarding; }

    /// <summary>
    /// ガード状態設定
    /// </summary>
    /// <param name="guarding">ガード状態</param>
    void SetGuarding(bool guarding) { m_isGuarding = guarding; }

    /// <summary>
    /// 前フレームでガードしていたかどうか
    /// </summary>
    /// <returns>true: 前フレームでガードしていた, false: 前フレームでガードしていなかった</returns>
    bool WasGuarding() const { return m_wasGuarding; }

    /// <summary>
    /// 前フレームでガードしていたかどうか設定
    /// </summary>
    /// <param name="was">前フレームでガードしていたかどうか</param>
    void SetWasGuarding(bool was) { m_wasGuarding = was; }

    /// <summary>
    /// ガードアニメーションタイマー取得
    /// </summary>
    /// <returns>ガードアニメーションタイマー値</returns>
    float GetGuardAnimTimer() const { return m_guardAnimTimer; }

    /// <summary>
    /// ガードアニメーション時間取得
    /// </summary>
    /// <returns>ガードアニメーション時間</returns>
    float GetGuardAnimDuration() const { return m_guardAnimDuration; }

    /// <summary>
    /// 盾の耐久値取得
    /// </summary>
    /// <returns>盾の耐久値</returns>
    float GetDurability() const { return m_shieldDurability; }

    /// <summary>
    /// 盾の最大耐久値取得
    /// </summary>
    /// <returns>盾の最大耐久値</returns>
    float GetMaxDurability() const { return m_maxShieldDurability; }

    /// <summary>
    /// 盾バーアニメーション値取得
    /// </summary>
    /// <returns>盾バーアニメーション値</returns>
    float GetBarAnim() const { return m_shieldBarAnim; }

    /// <summary>
    /// 盾が壊れているかどうか
    /// </summary>
    /// <returns>true: 壊れている, false: 壊れていない</returns>
    bool IsShieldBroken() const { return m_isShieldBroken; }

    /// <summary>
    /// 盾でダメージを受ける
    /// </summary>
    /// <param name="damage">ダメージ量</param>
    /// <param name="pEffect">エフェクトポインタ</param>
    /// <param name="pCamera">カメラポインタ</param>
    /// <param name="playerPos">プレイヤー位置ベクトル</param>
    /// <returns>残りのダメージ量</returns>
    float TakeDamage(float damage, Effect* pEffect, Camera* pCamera, const VECTOR& playerPos);

    /// <summary>
    /// ジャストガード判定
    /// </summary>
    /// <returns>true: ジャストガード成功, false: できなかった</returns>
    bool IsJustGuarded() const;

    /// <summary>
    /// ガードタイマー取得
    /// </summary>
    /// <returns>ガードタイマー値</returns>
    int GetGuardTimer() const { return m_guardTimer; }

    /// <summary>
    /// ガードエフェクト更新
    /// </summary>
    /// <param name="pEffect">エフェクトポインタ</param>
    /// <param name="pCamera">カメラポインタ</param>
    /// <param name="playerPos">プレイヤー位置ベクトル</param>
    /// <param name="isSwitchingWeapon">武器切り替え中かどうか</param>
    void UpdateGuardEffect(Effect* pEffect, Camera* pCamera, const VECTOR& playerPos, bool isSwitchingWeapon);

    /// <summary>
    /// スパークエフェクト更新
    /// </summary>
    /// <param name="pEffect">エフェクトポインタ</param>
    /// <param name="playerPos">プレイヤー位置ベクトル</param>
    /// <param name="pCamera">カメラポインタ</param>
    void UpdateSparkEffect(Effect* pEffect, const VECTOR& playerPos, Camera* pCamera);

    /// <summary>
    /// 盾のSwayオフセット取得
    /// </summary>
    /// <returns>Swayオフセットベクトル</returns>
    VECTOR GetShieldSwayOffset() const { return m_shieldSwayOffset; }

    /// <summary>
    /// 盾のSway回転オフセット取得
    /// </summary>
    /// <returns>Sway回転オフセットベクトル</returns>
    VECTOR GetShieldSwayRotOffset() const { return m_shieldSwayRotOffset; }

    /// <summary>
    /// 盾UI画像ハンドル取得
    /// </summary>
    /// <returns>盾UI画像ハンドル</returns>
    int GetShieldImageHandle() const;

    /// <summary>
    /// シールドソーを投げる
    /// </summary>
    /// <param name="pCamera">カメラポインタ</param>
    /// <param name="playerPos">プレイヤー位置</param>
    /// <returns>投げられたかどうか</returns>
    bool ThrowShield(Camera* pCamera, const VECTOR& playerPos);

    /// <summary>
    /// シールドソーの更新処理
    /// </summary>
    /// <param name="deltaTime">デルタタイム</param>
    /// <param name="pCamera">カメラポインタ</param>
    /// <param name="playerPos">プレイヤー位置</param>
    /// <param name="enemyList">敵のリスト</param>
    /// <param name="pEffect">エフェクトポインタ</param>
    /// <param name="isGuarding">ガード状態</param>
    /// <param name="wasGuarding">前フレームのガード状態</param>
    /// <param name="collisionData">ステージの当たり判定データ</param>
    void UpdateShieldThrow(float deltaTime, Camera* pCamera, const VECTOR& playerPos, const std::vector<class EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData, Effect* pEffect, bool isGuarding, bool wasGuarding);

    /// <summary>
    /// シールドソーの3D描画
    /// </summary>
    /// <param name="pCamera">カメラポインタ</param>
    /// <param name="playerPos">プレイヤー位置</param>
    void DrawShieldThrow(Camera* pCamera, const VECTOR& playerPos) const;

    /// <summary>
    /// 盾を即座にプレイヤーの位置に戻す
    /// </summary>
    /// <param name="playerPos">プレイヤー位置</param>
    void ImmediateReturnShield(const VECTOR& playerPos);

    /// <summary>
    /// シールドソーが投げられているかどうか
    /// </summary>
    /// <returns>投げられているならtrue</returns>
    bool IsShieldThrown() const { return m_isShieldThrown; }

private:
    int m_shieldModelHandle;
    int m_shieldImageHandle;
    int m_guardTimer;
    int m_guardEffectHandle;
    int m_sparkEffectHandle;
    int m_sparkEffectTimer;
    int m_shieldThrowHitEnemyId;  // このフレームでヒットした敵のID（重複ヒット防止用）
    int m_shieldReflectCount;     // 反射回数
    int m_boomerangTotalTime;     // ブーメランSEの総時間
    bool m_isBoomerangFading;     // フェードアウト中フラグ
    float m_boomerangFadeVolume;  // フェードアウト用音量倍率
    float m_shieldHitSECooldown;  // ヒットSE再生用クールタイム

    float m_shieldDurability;
    float m_shieldBarAnim;
    float m_maxShieldDurability;
    float m_shieldRegenRate;
    float m_guardAnimTimer;
    float m_guardAnimDuration;
    float m_shieldAnimTimer;
    float m_shieldAnimDuration;
    float m_idleSwayTimer;
    float m_shieldThrowDistance;  // 投げた距離
    float m_shieldThrowSpeed;     // シールドの移動速度
    float m_shieldThrowMaxRange;  // 最大投げ距離
    float m_shieldThrowDamage;    // シールドソーのダメージ
    float m_shieldThrowRotationTimer; // シールドの回転タイマー
    float m_shieldThrowCooldownTimer; // シールド投げのクールタイムタイマー
    float m_shieldThrowFailedAnimTimer;   // 失敗アニメーションタイマー

    bool m_isShieldBroken;
    bool m_isGuarding;
    bool m_wasGuarding;
    bool m_isShieldAnimating;
    bool m_isShieldRecovering;
    bool m_isTutorial;
    bool m_wasSwitchingWeapon; // 武器切り替え状態の記録（エフェクト再生用）
    bool m_isShieldThrown;
    bool m_isShieldThrowFailedAnimating;  // 失敗アニメーション中フラグ

    VECTOR m_shieldSwayOffset;
    VECTOR m_shieldSwayRotOffset;
    VECTOR m_shieldThrowPos;      // シールドの現在位置
    VECTOR m_shieldThrowDir;      // シールドの移動方向（カメラの前方方向）
    VECTOR m_shieldThrowStartPos; // 投げ始めた位置

    // シールドソー関連
    enum class ShieldThrowState
    {
        Idle,      // 待機中
        Throwing,  // 投げ中（前方移動）
        Returning  // 戻り中
    };
    ShieldThrowState m_shieldThrowState;
};
