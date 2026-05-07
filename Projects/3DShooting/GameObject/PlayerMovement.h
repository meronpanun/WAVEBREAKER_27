#pragma once
#include "EffekseerForDXLib.h"
#include "Stage.h"
#include <memory>
#include <vector>
#include <string>

class Camera;
class CapsuleCollider;

/// <summary>
/// プレイヤーの移動・物理管理クラス
/// </summary>
class PlayerMovement
{
public:
    PlayerMovement();
    ~PlayerMovement() = default;

    void Init(const VECTOR& pos, float moveSpeed, float runSpeed, float scale);
    void Update(float deltaTime, Camera* pCamera, bool isDead, bool isTackling,
        bool isFlightMode,
        const std::vector<Stage::StageCollisionData>& collisionData,
        bool isInputDisabled = false);

    /// <summary>
    /// 位置取得
    /// </summary>
    /// <returns>位置ベクトル</returns>
    VECTOR GetPos() const { return m_modelPos; }

    /// <summary>
    /// 位置設定
    /// </summary>
    /// <param name="pos">位置ベクトル</param>
    void SetPos(const VECTOR& pos) { m_modelPos = pos; }

    /// <summary>
    /// 移動中かどうか
    /// </summary>
    /// <returns>移動中ならtrue</returns>
    bool IsMoving() const { return m_isMoving; }

    /// <summary>
    /// 現在の移動速度を取得
    /// </summary>
    /// <returns>移動速度</returns>
    float GetCurrentSpeed() const { return m_currentSpeed; }

    /// <summary>
    /// ジャンプ中かどうか
    /// </summary>
    /// <returns>ジャンプ中ならtrue</returns>
    bool IsJumping() const { return m_isJumping; }

    /// <summary>
    /// 前フレームでジャンプ中だったかどうか
    /// </summary>
    /// <returns>true: ジャンプ中だった, false: ジャンプ中でなかった</returns>
    bool WasJumping() const { return m_wasJumping; }

    /// <summary>
    /// 前フレームで走っていたかどうか
    /// </summary>
    /// <returns>true: 走っていた, false: 走っていなかった</returns>
    bool WasRunning() const { return m_wasRunning; }

    /// <summary>
    /// 地面に接地しているかどうか
    /// </summary>
    /// <returns>接地していればtrue</returns>
    bool IsOnGround() const;

    /// <summary>
    /// 着地した瞬間かどうか
    /// </summary>
    bool JustLanded() const { return m_justLanded; }

    /// <summary>
    /// 着地時の垂直速度を取得
    /// </summary>
    float GetImpactVelocity() const { return m_impactVelocity; }

    /// <summary>
    /// コライダーを取得
    /// </summary>
    /// <returns>コライダーの共有ポインタ</returns>
    std::shared_ptr<CapsuleCollider> GetBodyCollider() const { return m_pBodyCollider; }

    /// <summary>
    /// 接地しているオブジェクト名を取得する
    /// </summary>
    /// <returns>接地しているオブジェクト名</returns>
    std::string GetGroundedObjectName() const { return m_groundedObjectName; }

    /// <summary>
    /// ジャンプ処理
    /// </summary>
    /// <param name="pCamera">カメラのポインタ</param>
    void Jump(Camera* pCamera);

    /// <summary>
    /// 地面のY座標を取得
    /// </summary>
    /// <returns>地面のY座標</returns>
    static float GetGroundY() { return 0.0f; }

    /// <summary>
    /// 地面接地判定の許容値
    /// </summary>
    /// <returns>許容値</returns>
    static float GetGroundCheckTolerance() { return 1.0f; }

    /// <summary>
    /// ジャンプ移動速度を取得
    /// </summary>
    /// <returns>ジャンプ移動速度</returns>
    VECTOR GetJumpMoveVelocity() const { return m_jumpMoveVelocity; }

    // ダッシュモード管理
    void CancelRunMode() { m_isRunMode = false; }
    bool IsRunMode() const { return m_isRunMode; }

    /// <summary>
    /// ノックバック力を適用
    /// </summary>
    /// <param name="velocity">ノックバック速度ベクトル</param>
    void ApplyKnockback(const VECTOR& velocity);

    static constexpr float kGroundY = 0.0f;
    
    // 強制的に垂直速度をリセット（ボスのシールドに乗った時など）
    void ResetVerticalVelocity() { m_jumpVelocity = 0.0f; }

private:
    void UpdateCollider();
    void UpdateFlightMode(float deltaTime, Camera* pCamera, bool isDead, bool isInputDisabled);
    void UpdateNormalMode(float deltaTime, Camera* pCamera, bool isDead, bool isTackling,
        const std::vector<Stage::StageCollisionData>& collisionData, bool isInputDisabled);

    void UpdateAirControl(const VECTOR& moveDir, Camera* pCamera, float timeScale);

    VECTOR CalculateMoveDirection(Camera* pCamera, const unsigned char* keyState);
    void HandleJump(const unsigned char* keyState, const unsigned char* prevKeyState, bool isDead,
        bool isTackling, const VECTOR& moveDir, Camera* pCamera);
    void HandlePhysics(bool isOnGround, Camera* pCamera);
    void ResolveCollisions(const std::vector<Stage::StageCollisionData>& collisionData, Camera* pCamera, bool isMoving);

private:
    VECTOR m_modelPos;
    VECTOR m_scale;
    VECTOR m_jumpMoveVelocity;
    VECTOR m_airSideControlVelocity; // 空中での左右操作速度
    VECTOR m_knockbackVelocity;      // ノックバック速度

    float m_moveSpeed;
    float m_runSpeed;
    float m_jumpVelocity;
    float m_jumpStartYaw;
    float m_jumpSpeedScalar;
    float m_coyoteTimeTimer; // コヨーテタイム用のタイマー
    float m_currentSpeed;

    // フラグ
    bool m_isMoving;
    bool m_isJumping;
    bool m_wasJumping;
    bool m_wasRunning;
    bool m_isGroundedOnStage;
    bool m_isRunMode;
    bool m_isRunJumping;
    bool m_isJumpInertiaActive;
    bool m_justLanded;      // 着地した瞬間フラグ
    float m_impactVelocity; // 着地時の衝撃速度
    float m_airborneTime;   // 滞空時間

    std::shared_ptr<CapsuleCollider> m_pBodyCollider;

    std::string m_groundedObjectName;
};
