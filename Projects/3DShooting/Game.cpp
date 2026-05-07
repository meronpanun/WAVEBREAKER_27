#include "Game.h"
#include "Player.h"
#include "SceneBase.h"
#include "SceneMain.h"
#include "SceneManager.h"
#include "DxLib.h"

// グローバルなカメラ感度
float Game::g_cameraSensitivity = 0.002f;
Player* Game::m_pPlayer = nullptr;
WaveManager* Game::m_pWaveManager = nullptr;
SceneManager* Game::m_pSceneManager = nullptr;

// 画面解像度とモードの初期化
int Game::m_screenWidth = 1920;
int Game::m_screenHeight = 1080;
int Game::m_colorBitNum = 32;
bool Game::s_isWindowMode = true;

// タイムスケール関連の初期化
float Game::g_timeScale = 1.0f;         // 初期値は通常速度
float Game::g_targetTimeScale = 1.0f;   // 目標タイムスケール
float Game::g_timeScaleDuration = 0.0f; // 持続時間
float Game::g_timeScaleTimer = 0.0f;    // タイマー
float Game::g_initialTimeScale = 1.0f;  // 初期化

void Game::SetResolution(int w, int h)
{
    m_screenWidth = w;
    m_screenHeight = h;
    SetGraphMode(m_screenWidth, m_screenHeight, m_colorBitNum);
    if (s_isWindowMode)
    {
        SetWindowSize(m_screenWidth, m_screenHeight);
    }
    SetDrawScreen(DX_SCREEN_BACK); // 描画先を裏画面に設定し直す
}

void Game::SetWindowMode(bool windowed)
{
    s_isWindowMode = windowed;
    ChangeWindowMode(s_isWindowMode);
    // 画面モード切替時はマウスカーソルを表示する（デバッグ操作用）
    SetMouseDispFlag(true);
}

void Game::SetTimeScale(float scale, float duration)
{
    g_timeScale = scale;
    g_initialTimeScale = scale; // 開始時のスケールを保存
    g_targetTimeScale = 1.0f;
    g_timeScaleDuration = duration;
    g_timeScaleTimer = duration;
}

void Game::UpdateTimeScale()
{
    if (g_timeScaleTimer > 0.0f)
    {
        // フレーム時間 (1/60) を減算。タイムスケールに関わらず実時間で管理
        g_timeScaleTimer -= 1.0f / 60.0f;

        if (g_timeScaleTimer <= 0.0f)
        {
            g_timeScaleTimer = 0.0f;
            g_timeScale = 1.0f;
        }
        else
        {
            // 時間経過で徐々に元に戻す
            float t = 1.0f - (g_timeScaleTimer / g_timeScaleDuration);

            // 線形補間 (Lerp)
            g_timeScale = g_initialTimeScale + (1.0f - g_initialTimeScale) * t;
        }
    }
    else
    {
        g_timeScale = 1.0f;
    }
}

float Game::GetTimeScale()
{
    if (s_isPaused) return 0.0f;
    return g_timeScale;
}

float Game::GetUIScale()
{
    // 画面の高さに基づいてスケールを計算 (基準: 720p)
    // 動画のUIサイズに合わせるため、全体的に少し小さくする (0.7倍)
    return (static_cast<float>(m_screenHeight) / 720.0f) * 0.7f;
}

bool Game::s_isPaused = false;

void Game::SetPaused(bool paused)
{
    s_isPaused = paused;
}

bool Game::IsPaused()
{
    return s_isPaused;
}
