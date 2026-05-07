#include "DebugUtil.h"
#include "EffekseerForDXLib.h"
#include "DebugMenu.h"
#include "Game.h"
#include "SceneManager.h"
#include "SceneMain.h"
#include <cstdarg>

bool DebugUtil::s_isVisible = false; // デバッグウィンドウの表示状態を管理する静的変数
DebugMenu DebugUtil::s_debugMenu;

// 3Dカプセルのデバッグ描画関数
void DebugUtil::DrawCapsule(const VECTOR& a, const VECTOR& b, float radius, int div, int color, bool fill)
{
    DrawCapsule3D(a, b, radius, div, color, color, fill);
}

// 3D球のデバッグ描画関数
void DebugUtil::DrawSphere(const VECTOR& center, float radius, int div, int color, bool fill)
{
    DrawSphere3D(center, radius, div, color, color, fill);
}

// 2Dメッセージを描画する関数
void DebugUtil::DrawMessage(int x, int y, unsigned int color, const std::string& msg)
{
    DrawString(x, y, msg.c_str(), color);
}

// 2Dフォーマット文字列を描画する関数
void DebugUtil::DrawFormat(int x, int y, unsigned int color, const char* format, ...)
{
    char buf[256];
    va_list args;                              // 可変引数リスト
    va_start(args, format);                    // 可変引数の初期化
    vsnprintf(buf, sizeof(buf), format, args); // フォーマット文字列をバッファに書き込む
    va_end(args);                              // 可変引数の終了
    DrawString(x, y, buf, color); // 描画
}

// ロゴスキップキーが押されたかどうかをチェックする関数
bool DebugUtil::IsSkipLogoKeyPressed()
{
    return CheckHitKey(KEY_INPUT_S) != 0;
}

void DebugUtil::ShowDebugWindow()
{
    // F1キーが押された瞬間に表示/非表示を切り替え
    static int prevF1 = 0;
    int nowF1 = CheckHitKey(KEY_INPUT_F1);
    if (nowF1 && !prevF1)
    {
        s_isVisible = !s_isVisible;

        if (s_isVisible)
        {
            // デバッグウィンドウを開く時は必ずマウスを表示
            SetMouseDispFlag(true);
        }
        else
        {
            // デバッグウィンドウを閉じる時は現在のシーンに応じてマウスの表示を決定
            if (Game::m_pSceneManager)
            {
                SceneBase* currentScene = Game::m_pSceneManager->GetCurrentScene();
                if (dynamic_cast<SceneMain*>(currentScene))
                {
                    SetMouseDispFlag(false); // ゲーム本編では非表示
                }
                else
                {
                    SetMouseDispFlag(true); // それ以外のシーンでは表示
                }
            }
        }
    }
    prevF1 = nowF1;

    if (!s_isVisible) return;

    // デバッグウィンドウの背景を半透明で描画
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, NULL);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
    DrawBox(0, 0, screenW, screenH, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    s_debugMenu.Update();
    s_debugMenu.Draw(40, 40);
}

bool DebugUtil::IsDebugWindowVisible()
{
    return s_isVisible;
}