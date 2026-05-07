#include "InputManager.h"
#include "EffekseerForDXLib.h"
#include "Game.h"
#include <DxLib.h>

// InputManagerのシングルトンインスタンスを取得
InputManager* InputManager::GetInstance()
{
    static InputManager instance;
    return &instance;
}

InputManager::InputManager()
    : m_mouseWheelRot(0)
{
    for (int i = 0; i < kLogNum; ++i)
    {
        mouseLog[i] = 0;
        mouseRightLog[i] = 0;
    }
}

void InputManager::Update()
{
    // マウスボタンの入力ログを更新
    for (int i = kLogNum - 1; i >= 1; i--)
    {
        mouseLog[i] = mouseLog[i - 1];
        mouseRightLog[i] = mouseRightLog[i - 1];
    }
    // 最新のマウスボタンの状態を取得
    mouseLog[0] = (GetMouseInput() & MOUSE_INPUT_LEFT) ? 1 : 0;
    mouseRightLog[0] = (GetMouseInput() & MOUSE_INPUT_RIGHT) ? 1 : 0;
    // マウスホイールの回転量を取得
    m_mouseWheelRot = ::GetMouseWheelRotVol();
}

// 現在のマウスカーソルのスクリーン座標を取得
Vec2 InputManager::GetMousePos()
{
    Vec2 mousePos{ 0,0 };
    int mouseX = 0;
    int mouseY = 0;
    // マウスのスクリーン座標を取得
    if (GetMousePoint(&mouseX, &mouseY) == -1)
    {
        return mousePos;
    }
    mousePos.x = static_cast<float>(mouseX);
    mousePos.y = static_cast<float>(mouseY);
    return mousePos;
}

// マウスの左ボタンが現在押されているかどうかを判定
bool InputManager::IsPressMouseLeft()
{
    return (mouseLog[0]);
}

// マウスの左ボタンが押された瞬間のフレームかどうかを判定
bool InputManager::IsTriggerMouseLeft()
{
    bool isNow = mouseLog[0];  // 現在の状態
    bool isLast = mouseLog[1]; // 1フレーム前の状態
    return (isNow && !isLast);
}

// マウスの左ボタンが離された瞬間のフレームかどうかを判定
bool InputManager::IsReleaseMouseLeft()
{
    bool isNow = mouseLog[0];  // 現在の状態
    bool isLast = mouseLog[1]; // 1フレーム前の状態
    return (!isNow && isLast);
}

// マウスの右ボタンが押された瞬間のフレームかどうかを判定
bool InputManager::IsTriggerMouseRight()
{
    bool isNow = mouseRightLog[0];  // 現在の状態
    bool isLast = mouseRightLog[1]; // 1フレーム前の状態
    return (isNow && !isLast);
}

// マウスの右ボタンが現在押されているかどうかを判定
bool InputManager::IsPressMouseRight()
{
    return (mouseRightLog[0]);
}

// マウスの移動量に基づいてカメラの回転角度を更新
void InputManager::UpdateCameraRotation(float& cameraYaw, float& cameraPitch, float sensitivity)
{
    // ウィンドウがアクティブでないか、
    // PrintScreenキー（SYSRQ）が押されている場合は、カメラ操作とマウスの固定を無効化する
    if (GetWindowActiveFlag() == FALSE || CheckHitKey(KEY_INPUT_SYSRQ))
    {
        return;
    }

    Vec2 mousePos = GetMousePos();

    // マウスの移動量に基づいてカメラの回転角度を更新
    cameraYaw   += (mousePos.x - Game::GetScreenWidth()  * 0.5f) * sensitivity;
    cameraPitch -= (mousePos.y - Game::GetScreenHeight() * 0.5f) * sensitivity;

    // カメラのピッチ角度を制限 (ジンバルロック防止のため89度)
    constexpr float limit = 89.0f * (DX_PI_F / 180.0f);
    if (cameraPitch >  limit) cameraPitch = limit;
    if (cameraPitch < -limit) cameraPitch = -limit;

    // マウスの位置を中央に戻す
    SetMousePoint(static_cast<int>(Game::GetScreenWidth() * 0.5f), static_cast<int>(Game::GetScreenHeight() * 0.5f));
}

// マウスホイールの回転量を取得
int InputManager::GetMouseWheelRotVol()
{
    return m_mouseWheelRot;
}