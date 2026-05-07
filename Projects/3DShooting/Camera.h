#pragma once
#include "EffekseerForDXLib.h"

/// <summary>
/// カメラクラス
/// </summary>
class Camera
{
public:
	Camera();
	virtual ~Camera();

	void Init();
	void Update(bool isInputDisabled = false);

	/// <summary>
	/// カメラの感度を設定 
	/// </summary>
	/// <param name="sensitivity">感度</param> 
	void SetSensitivity(float sensitivity);

	/// <summary>
	/// カメラの位置を取得
	/// </summary>
	/// <returns>カメラの位置</returns>
	VECTOR GetPos() const { return m_pos; }

	/// <summary>
	/// カメラの注視点を取得
	/// </summary>
	/// <returns>カメラの注視点</returns>
	VECTOR GetTarget() const { return m_target; }

	/// <summary>
	/// カメラのオフセットを取得
	/// </summary>
	/// <returns>カメラのオフセット</returns>
	VECTOR GetOffset() const { return m_offset; }

	/// <summary>
	/// カメラのオフセットを設定
	/// </summary>
	/// <param name="offset">カメラのオフセット</param>
	void SetOffset(const VECTOR& offset) { m_offset = offset; }

	/// <summary>
	/// カメラの位置を設定
	/// </summary>
	/// <param name="pos">カメラの位置</param>
	void SetPos(const VECTOR& pos) { m_pos = pos; }
	/// <summary>
	/// カメラの注視点を設定
	/// </summary>
	/// <param name="target">カメラの注視点</param>
	void SetTarget(const VECTOR& target) { m_target = target; }

	/// <summary>
	/// プレイヤーの位置を設定
	/// </summary>
	/// <param name="playerPos">プレイヤーの位置</param>
	void SetPlayerPos(const VECTOR& playerPos) { m_playerPos = playerPos; }

	/// <summary>
	/// カメラの回転角度を取得
	/// </summary>
	/// <returns>カメラの回転角度</returns>
	float GetYaw()   const { return m_yaw; }
	/// <summary>
	/// カメラのピッチ角度を取得
	/// </summary>
	/// <returns>カメラのピッチ角度</returns>
	float GetPitch() const { return m_pitch; }

	/// <summary>
	/// カメラの位置と注視点をDxLibに設定
	/// </summary>
	void SetCameraToDxLib();

	/// <summary>
	/// カメラの視野角(FOV)を設定
	/// </summary>
	/// <param name="fov">視野角</param>
	void SetFOV(float fov);

	/// <summary>
	/// 現在の視野角(FOV)を取得
	/// </summary>
	/// <returns>現在の視野角(FOV)</returns>
	float GetFOV() const;

	/// <summary>
	/// カメラの視野角(FOV)をデフォルトに戻す
	/// </summary>
	void ResetFOV();

	/// <summary>
	/// カメラのオフセットをデフォルトに戻す
	/// </summary>
	void ResetOffset();

	/// <summary>
	/// 目標FOVをセット
	/// </summary>
	/// <param name="fov">目標FOV</param>
	void SetTargetFOV(float fov);

	/// <summary>
	/// シェイク効果を適用
	/// </summary>
	/// <param name="intensity">シェイクの強度</param>
	/// <param name="duration">シェイクの持続時間</param>
	void Shake(float intensity, float duration);

	/// <summary>
	/// シェイクオフセットを取得
	/// </summary>
	/// <returns>シェイクオフセット</returns>
	VECTOR GetShakeOffset() const { return m_shakeOffset; }

	/// <summary>
	/// Head Bobbingオフセットを取得
	/// </summary>
	/// <returns>Head Bobbingオフセット</returns>
	VECTOR GetHeadBobOffset() const { return m_headBobOffset; }

	/// <summary>
	/// Head Bobbin（歩行時の画面揺れ）の状態を設定
	/// </summary>
	/// <param name="isMoving">移動中かどうか</param>
	/// <param name="isRunning">走行中かどうか</param>
	void SetHeadBobbingState(bool isMoving, bool isRunning);

	/// <summary>
	/// 着地時の揺れを適用
	/// </summary>
	/// <param name="intensity">揺れの強さ</param>
	void ApplyLandingSway(float intensity);

	/// <summary>
	/// 着地時の揺れオフセットを取得
	/// </summary>
	/// <returns>着地時の揺れオフセット</returns>
	VECTOR GetLandingSwayOffset() const;

	/// <summary>
	/// ジャンプ時の揺れを適用
	/// </summary>
	/// <param name="intensity">揺れの強さ</param>
	void ApplyJumpSway(float intensity);

	/// <summary>
	/// ジャンプ時の揺れオフセットを取得
	/// </summary>
	/// <returns>ジャンプ時の揺れオフセット</returns>
	VECTOR GetJumpSwayOffset() const;
	
	/// <summary>
	/// Yawの差分を取得
	/// </summary>
	/// <returns>Yawの差分</returns>
	float GetYawDelta() const { return m_yawDelta; }

	/// <summary>
	/// 死亡アニメーションを再生
	/// </summary>
	/// <param name="timer">アニメーションタイマー</param>
	void PlayDeathAnimation(float timer);
	
private:
	/// <summary>
	/// Head Bobbin効果を更新
	/// </summary>
	void UpdateHeadBobbing();
	
	/// <summary>
	/// Sway効果を更新
	/// </summary>
	void UpdateSway();
	
	/// <summary>
	/// 値を滑らかに補間する
	/// </summary>
	/// <param name="current">現在の値</param>
	/// <param name="target">目標値</param>
	/// <param name="speed">補間速度</param>
	/// <returns>補間された値</returns>
	float Lerp(float current, float target, float speed);
	
private:
	// 位置・注視点管理
	VECTOR m_pos;            // カメラの位置
	VECTOR m_target;         // カメラの注視点
	VECTOR m_offset;         // カメラのオフセット
	VECTOR m_defaultOffset;  // デフォルトのオフセット
	VECTOR m_playerPos;      // プレイヤーの位置
	
	// 回転管理
	float m_yaw;             // ヨー角度
	float m_pitch;           // ピッチ角度
	float m_roll;            // ロール角度
	float m_sensitivity;     // カメラの感度
	float m_prevYaw;         // 前フレームのヨー角度
	float m_yawDelta;        // ヨー角度の差分
	
	// 視野角(FOV)管理
	float m_fov;             // カメラの視野角
	float m_defaultFov;      // デフォルトのFOV
	float m_targetFov;       // 目標FOV
	float m_fovLerpSpeed;    // FOVの補間速度
	
	// シェイク効果管理
	VECTOR m_shakeOffset;    // シェイクオフセット
	float  m_shakeIntensity; // シェイクの強度
	int    m_shakeDuration;  // シェイクの持続時間
	
	// Head Bobbing管理
	VECTOR m_headBobOffset;      // Head Bobbingによるオフセット
	float  m_headBobTimer;       // Head Bobbingのタイマー
	float  m_headBobIntensity;   // 現在のHead Bobbing強度
	float  m_targetBobIntensity; // 目標のHead Bobbing強度
	float  m_headBobSpeed;       // 現在のHead Bobbing速度
	float  m_targetBobSpeed;     // 目標のHead Bobbing速度
	bool   m_isMoving;           // 移動中かどうか
	bool   m_isRunning;          // 走行中かどうか
	
	// 着地時の揺れ管理
	VECTOR m_landingSwayOffset; // 着地時の揺れオフセット
	float m_landingSwayTimer;
	float m_landingSwayIntensity;
	
	// ジャンプ時の揺れ管理
	VECTOR m_jumpSwayOffset; // ジャンプ時の揺れオフセット
	float m_jumpSwayTimer;
	float m_jumpSwayIntensity;
	
	// Sway管理
	VECTOR m_swayOffset;
	VECTOR m_swayRotOffset;

	// 死亡アニメーション管理
	bool m_isDeathAnimationPlaying; // 死亡アニメーション再生中か
	float m_deathAnimationTimer;    // 死亡アニメーションのタイマー
	bool m_hasBounced;              // バウンドしたか
};