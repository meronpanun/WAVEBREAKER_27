#include "ScoreUI.h"
#include "Game.h"
#include "ScoreManager.h"
#include <cstdarg>

ScoreUI::ScoreUI()
    : m_totalScorePopupValue(0)
    , m_totalScorePopupTimer(0.0f)
    , m_scoreFont(-1)
    , m_prevCombo(0)
    , m_comboPulseScale(1.0f)
    , m_comboAnimTimer(0.0f)
{
}

ScoreUI::~ScoreUI()
{
    if (m_scoreFont != -1)
    {
        DeleteFontToHandle(m_scoreFont);
    }
}

void ScoreUI::Init()
{
    m_scoreFont = CreateFontToHandle("Arial Black", 24, 4, DX_FONTTYPE_ANTIALIASING_EDGE_8X8);
    
    // シーン遷移時に前回の状態が残らないようリセット
    ScoreManager::Instance().ResetAll();
    m_prevCombo = 0;
    m_comboPulseScale = 1.0f;
    m_comboAnimTimer = 0.0f;
    m_popups.clear();
    m_totalScorePopupValue = 0;
    m_totalScorePopupTimer = 0.0f;
}

void ScoreUI::Update(float deltaTime)
{
    float timeScale = Game::GetTimeScale();
    
    // ポップアップタイマー更新
    for (auto it = m_popups.begin(); it != m_popups.end(); )
    {
        it->timer -= 1.0f * timeScale;
        if (it->timer <= 0)
        {
            it = m_popups.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (m_totalScorePopupTimer > 0)
    {
        m_totalScorePopupTimer -= 1.0f * timeScale;
        if (m_totalScorePopupTimer <= 0)
        {
            m_totalScorePopupValue = 0;
        }
    }

    // コンボアニメーション更新
    int currentCombo = ScoreManager::Instance().GetCombo();
    if (currentCombo > m_prevCombo && currentCombo > 0)
    {
        // コンボが増えた瞬間の演出
        m_comboPulseScale = 1.3f; 
        m_comboAnimTimer = 20.0f;
    }
    m_prevCombo = currentCombo;

    if (currentCombo > 0)
    {
        // ベースとなるスケール（コンボ数に応じてじわじわ大きくする）
        float baseScale = 1.0f + (currentCombo * 0.02f);
        if (baseScale > 1.8f) baseScale = 1.8f;

        // パルス（増えた瞬間の膨らみ）をベーススケールへ戻していく
        m_comboPulseScale += (baseScale - m_comboPulseScale) * 0.2f * timeScale;
        
        if (m_comboAnimTimer > 0) m_comboAnimTimer -= 1.0f * timeScale;
    }
    else
    {
        m_comboPulseScale = 1.0f;
        m_comboAnimTimer = 0.0f;
    }
}

void ScoreUI::AddScorePopup(int value, bool isHeadShot)
{
    m_popups.push_back({ value, isHeadShot, kScorePopupDuration, kScorePopupDuration });
    m_totalScorePopupValue += value;
    m_totalScorePopupTimer = kScorePopupDuration;
}

void ScoreUI::Draw()
{
    if (m_popups.empty() && m_totalScorePopupTimer <= 0 && ScoreManager::Instance().GetCombo() < 2) return;

    float scale = Game::GetUIScale();
    int fontSize = 32;
    int scaledPopupOffsetY = static_cast<int>(fontSize * 1.2f * scale);
    
    int popupBaseX = Game::GetScreenWidth() * 0.5f + static_cast<int>(kScorePopupX * scale);
    int popupBaseY = Game::GetScreenHeight() * 0.5f + static_cast<int>(kScorePopupY * scale);
    
    int idx = 0;
    float lastComboRate = ScoreManager::Instance().GetLastComboRate();
    int displayCombo = (ScoreManager::Instance().GetCombo() > 1) ? ScoreManager::Instance().GetCombo() : 1;

    int alpha = 255;
    if (m_totalScorePopupTimer < 20)
    {
        alpha = static_cast<int>(255 * m_totalScorePopupTimer / 20.0f);
    }
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    auto DrawShadowText = [&](int x, int y, unsigned int color, const char* format, ...) {
        va_list args;
        va_start(args, format);
        char buf[256];
        vsprintf_s(buf, format, args);
        va_end(args);
        DrawStringToHandle(x + 2, y + 2, buf, 0x000000, m_scoreFont);
        DrawStringToHandle(x, y, buf, color, m_scoreFont);
    };

    // 合計スコア
    if (m_totalScorePopupTimer > 0)
    {
        if (lastComboRate > 1.0f)
        {
            DrawShadowText(popupBaseX, popupBaseY + idx * scaledPopupOffsetY, 0x00ffcc, "+%d (×%.2f)", m_totalScorePopupValue, lastComboRate);
        }
        else
        {
            DrawShadowText(popupBaseX, popupBaseY + idx * scaledPopupOffsetY, 0x00ffcc, "+%d", m_totalScorePopupValue);
        }
        idx++;
    }

    // 内訳
    if (!m_popups.empty())
    {
        int lastIsHeadShot = -1;
        for (const auto& popup : m_popups)
        {
            if (lastIsHeadShot == -1 || lastIsHeadShot != static_cast<int>(popup.isHeadShot))
            {
                if (popup.timer < 20) {
                    int detailAlpha = static_cast<int>(alpha * popup.timer / 20.0f);
                    SetDrawBlendMode(DX_BLENDMODE_ALPHA, detailAlpha);
                }

                if (popup.isHeadShot)
                {
                    DrawShadowText(popupBaseX, popupBaseY + idx * scaledPopupOffsetY, 0xffd700, "200pt HEADSHOT ×%d", displayCombo);
                }
                else
                {
                    DrawShadowText(popupBaseX, popupBaseY + idx * scaledPopupOffsetY, 0xeeeeee, "100pt ZOMBIE KILL ×%d", displayCombo);
                }
                idx++;
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            }
            lastIsHeadShot = static_cast<int>(popup.isHeadShot);
        }
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // --- コンボ情報の描画 ---
    int comboCount = ScoreManager::Instance().GetCombo();
    if (comboCount >= 2) // 2コンボ以上で表示
    {
        int comboX = Game::GetScreenWidth() * 0.5f + static_cast<int>(kScorePopupX * scale);
        int comboY = Game::GetScreenHeight() * 0.5f + static_cast<int>((kScorePopupY - 80) * scale);

        // 1. タイマーとアニメーションの計算
        int timer = ScoreManager::Instance().GetComboTimer();
        float timerRatio = static_cast<float>(timer) / ScoreManager::kComboGraceFrame;
        
        // 基本の色設定
        unsigned int comboColor = 0xffffff;
        if (comboCount >= 8) comboColor = 0xffaa00; // オレンジ
        else if (comboCount >= 3) comboColor = 0xffff00; // 黄色
        else if (comboCount >= 1) comboColor = 0x00ffff; // 水色

        unsigned int barColor = (timerRatio > 0.3f) ? 0x00ffff : 0xff3333;

        float warningPulse = 0.0f;
        if (timerRatio < 0.5f)
        {
            float intensity = (0.5f - timerRatio) / 0.5f; 
            float blinkPhase = (GetNowCount() % 400) / 400.0f; 
            float t = (sinf(blinkPhase * DX_PI_F * 2.0f) + 1.0f) * 0.5f; 
            
            warningPulse = t * 0.2f * intensity; 
            
            int baseR = (comboColor >> 16) & 0xff;
            int baseG = (comboColor >> 8) & 0xff;
            int baseB = comboColor & 0xff;
            int targetR = 255;
            int targetG = 30;
            int targetB = 30;
            
            float blend = (t * 0.7f + 0.3f) * intensity;
            if (blend > 1.0f) blend = 1.0f;

            int r = static_cast<int>(baseR + (targetR - baseR) * blend);
            int g = static_cast<int>(baseG + (targetG - baseG) * blend);
            int b = static_cast<int>(baseB + (targetB - baseB) * blend);
            comboColor = GetColor(r, g, b);
            
            barColor = GetColor(255, (int)(255 * (1.0f - intensity)), (int)(255 * (1.0f - intensity)));
        }

        // 2. ゲージ描画
        int barW = static_cast<int>(150 * scale);
        int barH = static_cast<int>(6 * scale);
        int barX = comboX;
        int barY = comboY + static_cast<int>(45 * scale);

        DrawBox(barX, barY, barX + barW, barY + barH, 0x333333, true);
        DrawBox(barX, barY, barX + static_cast<int>(barW * timerRatio), barY + barH, barColor, true);

        // 3. コンボ数テキスト描画
        char comboBuf[64];
        sprintf_s(comboBuf, "%d COMBO", comboCount);
        int baseFontSize = 24;
        float currentScale = m_comboPulseScale + warningPulse;
        int textWidth = GetDrawStringWidthToHandle(comboBuf, (int)strlen(comboBuf), m_scoreFont);
        int drawX = comboX - static_cast<int>(textWidth * (currentScale - 1.0f) * 0.5f);
        int drawY = comboY - static_cast<int>(baseFontSize * (currentScale - 1.0f) * 0.5f);

        SetDrawMode(DX_DRAWMODE_BILINEAR);
        int shadowOffset = static_cast<int>(2 * currentScale);
        // 影
        DrawRotaStringToHandle(drawX + shadowOffset, drawY + shadowOffset, currentScale, currentScale, 0.0, 0.0, 0.0, 0x000000, m_scoreFont, 0, false, comboBuf);
        // 本体
        DrawRotaStringToHandle(drawX, drawY, currentScale, currentScale, 0.0, 0.0, 0.0, comboColor, m_scoreFont, 0, false, comboBuf);
        SetDrawMode(DX_DRAWMODE_NEAREST);
    }
}
