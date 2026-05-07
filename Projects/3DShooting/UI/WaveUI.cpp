#include "WaveUI.h"
#include "DebugUtil.h"
#include "WaveManager.h"
#include "Game.h"
#include <algorithm>
#include <cstdio>

namespace
{
    // ウェーブ画像の描画幅
    constexpr int kWaveImageDrawWidth = 150;

    // デパック情報の表示位置
    constexpr int kDebugInfoPosY = 10;     // 高さ
    constexpr int kDebugInfoSpacing = 100; // 項目間の間隔
    constexpr int kDebugInfoPosX = 10;     // 左端からのX座標
    constexpr int kFontSize = 16;          // フォントサイズ
}

WaveUI::WaveUI(WaveManager* waveManager)
    : m_pWaveManager(waveManager)
    , m_waveImageAnimTimer(0)
    , m_waveImageAnimDuration(45)
    , m_waveImageAnimHoldDuration(30)
    , m_waveImageAnimInitialHoldDuration(30)
    , m_isWaveImageAnimating(false)
{
    for (int i = 0; i < 5; ++i)
    {
        m_waveImages[i] = -1;
    }
}

WaveUI::~WaveUI()
{
    for (int i = 0; i < 5; ++i)
    {
        if (m_waveImages[i] >= 0)
        {
            DeleteGraph(m_waveImages[i]);
            m_waveImages[i] = -1;
        }
    }
}

void WaveUI::Init()
{
    // ウェーブ画像の読み込み
    m_waveImages[0] = LoadGraph("data/image/wave1.png");
    m_waveImages[1] = LoadGraph("data/image/wave2.png");
    m_waveImages[2] = LoadGraph("data/image/wave3.png");
    m_waveImages[3] = LoadGraph("data/image/wave4.png");
    m_waveImages[4] = LoadGraph("data/image/wave5.png");
}

void WaveUI::Update(float deltaTime)
{
    // ウェーブマネージャーの状態を監視して演出を開始
    if (m_pWaveManager && m_pWaveManager->GetState() == WaveManager::WaveState::Starting && !m_isWaveImageAnimating)
    {
        StartWaveAnimation();
    }

    // ウェーブ画像アニメーションの更新
    if (m_isWaveImageAnimating)
    {
        // タイムスケールを考慮した更新
        float increment = 1.0f * Game::GetTimeScale();
        m_waveImageAnimTimer += static_cast<int>(increment);
        
        if (m_waveImageAnimTimer >= m_waveImageAnimInitialHoldDuration + m_waveImageAnimDuration + m_waveImageAnimHoldDuration)
        {
            m_isWaveImageAnimating = false;
        }
    }
}

void WaveUI::StartWaveAnimation()
{
    m_isWaveImageAnimating = true;
    m_waveImageAnimTimer = 0;
}

void WaveUI::Draw()
{
    if (!m_pWaveManager) return;

    int currentWave = m_pWaveManager->GetCurrentWave();
    bool isWaveActive = m_pWaveManager->IsWaveActive();
    bool isAllWavesCompleted = m_pWaveManager->IsAllWavesCompleted();

    // ウェーブ中は常に画像を表示
    if (!isAllWavesCompleted && currentWave >= 1 && currentWave <= 5 && (m_isWaveImageAnimating || isWaveActive))
    {
        int img = m_waveImages[currentWave - 1];
        if (img == -1) return;

        int imgW = 0, imgH = 0;
        GetGraphSize(img, &imgW, &imgH);
        int screenW = 0, screenH = 0;
        GetScreenState(&screenW, &screenH, NULL);

        // 最終的な位置とサイズ
        int targetDrawW = kWaveImageDrawWidth;
        int targetDrawH = imgH * targetDrawW / imgW;
        int targetX = (screenW - targetDrawW) * 0.5f;
        int targetY = 0; // 画面上部中央

        // 初期位置とサイズ (拡大して中央)
        int startDrawW = static_cast<int>(screenW * 0.4f);
        int startDrawH = imgH * startDrawW / imgW;
        int startX = (screenW - startDrawW) * 0.5f; // 画面中央
        int startY = (screenH - startDrawH) * 0.5f; // 画面中央

        int currentX, currentY, currentDrawW, currentDrawH;

        if (m_isWaveImageAnimating)
        {
            float t;
            if (m_waveImageAnimTimer < m_waveImageAnimInitialHoldDuration)
            {
                t = 0.0f; // 初期ホールド中は補間を0に固定
            }
            else if (m_waveImageAnimTimer < m_waveImageAnimInitialHoldDuration + m_waveImageAnimDuration)
            {
                t = (float)(m_waveImageAnimTimer - m_waveImageAnimInitialHoldDuration) / m_waveImageAnimDuration;
            }
            else
            {
                t = 1.0f; // ホールド中は補間を終了位置に固定
            }
            t = (std::min)(1.0f, t); // 0.0fから1.0fにクランプ

            // 線形補間
            currentX = static_cast<int>(startX + (targetX - startX) * t);
            currentY = static_cast<int>(startY + (targetY - startY) * t);
            currentDrawW = static_cast<int>(startDrawW + (targetDrawW - startDrawW) * t);
            currentDrawH = static_cast<int>(startDrawH + (targetDrawH - startDrawH) * t);
        }
        else
        {
            currentX = targetX;
            currentY = targetY;
            currentDrawW = targetDrawW;
            currentDrawH = targetDrawH;
        }

        DrawExtendGraph(currentX, currentY, currentX + currentDrawW, currentY + currentDrawH, img, true);
    }
}

void WaveUI::DrawDebugSpawnAreas(const std::vector<SpawnAreaInfo>& spawnAreaList, bool isTutorial)
{
    for (const auto& area : spawnAreaList)
    {
        // 現在のモード（Tutorial/Main）と一致するエリアのみ描画
        if ((isTutorial && area.type == 1) || (!isTutorial && area.type == 0))
        {
            // ワイヤーフレームの色設定
            unsigned int color;
            if (area.type == 0) // Main
            {
                // 高さタイプによって色を変える
                if (std::abs(area.center.y - 200.0f) < 10.0f)
                {
                    color = 0x00ff00; // 緑 (下段)
                }
                else if (std::abs(area.center.y - 562.0f) < 10.0f)
                {
                    color = 0xffff00; // 黄 (中段)
                }
                else if (std::abs(area.center.y - 962.0f) < 10.0f)
                {
                    color = 0xff0000; // 赤 (上段)
                }
                else
                {
                    color = 0x00ffff; // シアン (その他)
                }
            }
            else // Tutorial
            {
                color = 0xff00ff; // マゼンタ
            }

            // 直方体の頂点を計算して描画
            VECTOR minPos = VSub(area.center, VScale(area.size, 0.5f));
            VECTOR maxPos = VAdd(area.center, VScale(area.size, 0.5f));

            // 上面
            DrawLine3D(VGet(minPos.x, maxPos.y, minPos.z),
                       VGet(maxPos.x, maxPos.y, minPos.z), color);
            DrawLine3D(VGet(maxPos.x, maxPos.y, minPos.z),
                       VGet(maxPos.x, maxPos.y, maxPos.z), color);
            DrawLine3D(VGet(maxPos.x, maxPos.y, maxPos.z),
                       VGet(minPos.x, maxPos.y, maxPos.z), color);
            DrawLine3D(VGet(minPos.x, maxPos.y, maxPos.z),
                       VGet(minPos.x, maxPos.y, minPos.z), color);

            // 下面
            DrawLine3D(VGet(minPos.x, minPos.y, minPos.z),
                       VGet(maxPos.x, minPos.y, minPos.z), color);
            DrawLine3D(VGet(maxPos.x, minPos.y, minPos.z),
                       VGet(maxPos.x, minPos.y, maxPos.z), color);
            DrawLine3D(VGet(maxPos.x, minPos.y, maxPos.z),
                       VGet(minPos.x, minPos.y, maxPos.z), color);
            DrawLine3D(VGet(minPos.x, minPos.y, maxPos.z),
                       VGet(minPos.x, minPos.y, minPos.z), color);

            // 縦線
            DrawLine3D(VGet(minPos.x, minPos.y, minPos.z),
                       VGet(minPos.x, maxPos.y, minPos.z), color);
            DrawLine3D(VGet(maxPos.x, minPos.y, minPos.z),
                       VGet(maxPos.x, maxPos.y, minPos.z), color);
            DrawLine3D(VGet(maxPos.x, minPos.y, maxPos.z),
                       VGet(maxPos.x, maxPos.y, maxPos.z), color);
            DrawLine3D(VGet(minPos.x, minPos.y, maxPos.z),
                       VGet(minPos.x, maxPos.y, maxPos.z), color);
        }
    }
}
