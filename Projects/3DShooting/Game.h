#pragma once
#include <vector>

class Player;
class SceneManager;
class WaveManager;

/// <summary>
/// ゲームの基本情報を定義するクラス
/// </summary>
class Game
{
public:
    // 画面情報を定数定義
    static int m_screenWidth;
    static int m_screenHeight;
    static int m_colorBitNum;
    static bool s_isWindowMode;

    static void SetResolution(int w, int h);
    static int GetScreenWidth() { return m_screenWidth; }
    static int GetScreenHeight() { return m_screenHeight; }

    static void SetWindowMode(bool windowed);
    static bool IsWindowMode() { return s_isWindowMode; }

    // ウインドウのタイトル
    static constexpr const char* kWindowTitle = "WAVEBREAKER";

    static void SetTimeScale(float scale, float duration);
    static void UpdateTimeScale();
    static float GetTimeScale();
    static float GetUIScale();

    // チュートリアル用の一時停止機能
    static void SetPaused(bool paused);
    static bool IsPaused();

    // プレイヤーへのポインタ
    static Player* m_pPlayer;
    static WaveManager* m_pWaveManager;

    static SceneManager* m_pSceneManager;

    // グローバルなカメラ感度
    static float g_cameraSensitivity;

private:
    // タイムスケール関連
    static float g_timeScale;
    static float g_targetTimeScale;
    static float g_timeScaleDuration;
    static float g_timeScaleTimer;
    static float g_initialTimeScale; // 補間開始時のスケール

    static bool s_isPaused; // 一時停止フラグ
};
