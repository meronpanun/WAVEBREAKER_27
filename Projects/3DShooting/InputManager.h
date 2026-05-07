#pragma once
#include "Vec2.h"

/// <summary>
/// 入力管理クラス (シングルトン)
/// マウスやキーボードからの入力を一元的に管理
/// </summary>
class InputManager
{
public:
    /// <summary>
    /// InputManagerのシングルトンインスタンスを取得
    /// </summary>
    /// <returns>InputManagerのインスタンスへのポインタ</returns>
    static InputManager* GetInstance();

    /// <summary>
    /// 毎フレームの入力状態を更新
    /// マウスボタンの状態やホイールの回転量などを取得し、内部に保持する
    /// </summary>
    void Update();

    /// <summary>
    /// 現在のマウスカーソルのスクリーン座標を取得
    /// </summary>
    /// <returns>マウスカーソルのX, Y座標を格納したVec2オブジェクト</returns>
    Vec2 GetMousePos();

    /// <summary>
    /// マウスの左ボタンが現在押されているかどうかを判定
    /// </summary>
    /// <returns>押されていればtrue、そうでなければfalse</returns>
    bool IsPressMouseLeft();

    /// <summary>
    /// マウスの左ボタンが押された瞬間のフレームかどうかを判定
    /// </summary>
    /// <returns>押された瞬間であればtrue、そうでなければfalse</returns>
    bool IsTriggerMouseLeft();

    /// <summary>
    /// マウスの左ボタンが離された瞬間のフレームかどうかを判定
    /// </summary>
    /// <returns>離された瞬間であればtrue、そうでなければfalse</returns>
    bool IsReleaseMouseLeft();

    /// <summary>
    /// マウスの右ボタンが押された瞬間のフレームかどうかを判定
    /// </summary>
    /// <returns>押された瞬間であればtrue、そうでなければfalse</returns>
    bool IsTriggerMouseRight();

    /// <summary>
    /// マウスの右ボタンが現在押されているかどうかを判定
    /// </summary>
    /// <returns>押されていればtrue、そうでなければfalse</returns>
    bool IsPressMouseRight();

    /// <summary>
    /// マウスの移動量に基づいてカメラの回転角度を更新
    /// </summary>
    /// <param name="cameraYaw">更新するカメラのヨー角度への参照</param>
    /// <param name="cameraPitch">更新するカメラのピッチ角度への参照</param>
    /// <param name="sensitivity">マウス感度</param>
    void UpdateCameraRotation(float& cameraYaw, float& cameraPitch, float sensitivity);

    /// <summary>
    /// マウスホイールの回転量を取得
    /// </summary>
    /// <returns>マウスホイールの回転量 (正の値:上方向, 負の値:下方向)</returns>
    int GetMouseWheelRotVol();

private:
    /// <summary>
    /// InputManagerクラスのコンストラクタ
    /// シングルトンパターンにより外部からの直接生成はできない
    /// </summary>
    InputManager();
    
    /// <summary>
    /// コピーコンストラクタを削除し、シングルトン性を保証する
    /// </summary>
    InputManager(const InputManager&) = delete;
    
    /// <summary>
    /// 代入演算子を削除し、シングルトン性を保証する
    /// </summary>
    InputManager& operator=(const InputManager&) = delete;

    /// <summary>
    /// マウスボタンの入力履歴を保持する配列
    /// mouseLog[0]が最新の状態を示す
    /// </summary>
    static constexpr int kLogNum = 16;
    int mouseLog[kLogNum];      /// 左ボタンの入力ログ
    int mouseRightLog[kLogNum]; /// 右ボタンの入力ログ
    int m_mouseWheelRot;        /// マウスホイールの回転量
};