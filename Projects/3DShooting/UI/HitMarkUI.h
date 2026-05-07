#pragma once
#include "UIBase.h"
#include "DxLib.h"
#include "EnemyBase.h"

/// <summary>
/// ヒットマークUIクラス
/// </summary>
class HitMarkUI : public UIBase
{
public:
    HitMarkUI();
    virtual ~HitMarkUI();

    void Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

    /// <summary>
    /// ヒットマークを表示
    /// </summary>
    /// <param name="type">ヒット箇所</param>
    /// <param name="distance">敵との距離</param>
    void Trigger(EnemyBase::HitPart type, float distance);

private:
    float m_timer;
    float m_distance;
    EnemyBase::HitPart m_type;

    static constexpr float kDuration = 20.0f;
    static constexpr int kLineLength = 12;
    static constexpr int kCenterSpacing = 4;
    static constexpr int kLineThickness = 2;
    static constexpr int kDoubleLineOffset = 2;
};
