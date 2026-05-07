#include "HitMarkUI.h"
#include "Game.h"
#include <algorithm>

namespace
{
    // 距離減衰
    constexpr float kMinDistance = 300.0f;
    constexpr float kMaxDistance = 2000.0f;
    constexpr float kMaxRatio = 1.0f;
    constexpr float kMinRatio = 0.2f;
}

HitMarkUI::HitMarkUI()
    : m_timer(0.0f)
    , m_distance(0.0f)
    , m_type(EnemyBase::HitPart::Body)
{
}

HitMarkUI::~HitMarkUI()
{
}

void HitMarkUI::Init()
{
}

void HitMarkUI::Update(float deltaTime)
{
    if (m_timer > 0)
    {
        m_timer -= 1.0f * Game::GetTimeScale();
    }
}

void HitMarkUI::Trigger(EnemyBase::HitPart type, float distance)
{
    m_type = type;
    m_distance = distance;
    m_timer = kDuration;
}

void HitMarkUI::Draw()
{
    if (m_timer <= 0) return;

    float ratio = 1.0f;
    if (m_distance > kMinDistance)
    {
        float t = (m_distance - kMinDistance) / (kMaxDistance - kMinDistance);
        t = (std::min)(t, 1.0f);
        ratio = kMaxRatio * (1.0f - t) + kMinRatio * t;
    }

    int alpha = static_cast<int>((255 * m_timer * ratio) / kDuration);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    unsigned int color = (m_type == EnemyBase::HitPart::Head) ? 0xffd700 : 0xff4500;

    float scale = Game::GetUIScale();
    int scaledLineLength = static_cast<int>(kLineLength * scale);
    int scaledCenterSpacing = static_cast<int>(kCenterSpacing * scale);
    int scaledThickness = (std::max)(1, static_cast<int>(kLineThickness * scale));
    int centerX = static_cast<int>(Game::GetScreenWidth() * 0.5f);
    int centerY = static_cast<int>(Game::GetScreenHeight() * 0.5f);
    
    // 通常
    DrawLine(centerX - scaledLineLength, centerY - scaledLineLength, centerX - scaledCenterSpacing, centerY - scaledCenterSpacing, color, scaledThickness);
    DrawLine(centerX + scaledCenterSpacing, centerY + scaledCenterSpacing, centerX + scaledLineLength, centerY + scaledLineLength, color, scaledThickness);
    DrawLine(centerX - scaledLineLength, centerY + scaledLineLength, centerX - scaledCenterSpacing, centerY + scaledCenterSpacing, color, scaledThickness);
    DrawLine(centerX + scaledCenterSpacing, centerY - scaledCenterSpacing, centerX + scaledLineLength, centerY - scaledLineLength, color, scaledThickness);

    // ヘッドショット二重線
    if (m_type == EnemyBase::HitPart::Head)
    {
        int offset = static_cast<int>(kDoubleLineOffset * scale);
        DrawLine(centerX - scaledLineLength - offset, centerY - scaledLineLength + offset, centerX - scaledCenterSpacing - offset, centerY - scaledCenterSpacing + offset, color, scaledThickness);
        DrawLine(centerX + scaledCenterSpacing + offset, centerY + scaledCenterSpacing - offset, centerX + scaledLineLength + offset, centerY + scaledLineLength - offset, color, scaledThickness);
        DrawLine(centerX - scaledLineLength - offset, centerY + scaledLineLength - offset, centerX - scaledCenterSpacing - offset, centerY + scaledCenterSpacing - offset, color, scaledThickness);
        DrawLine(centerX + scaledCenterSpacing + offset, centerY - scaledCenterSpacing + offset, centerX + scaledLineLength + offset, centerY - scaledLineLength + offset, color, scaledThickness);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
