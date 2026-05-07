#pragma once
#include "UIBase.h"
#include <vector>
#include <string>
#include "DxLib.h"

/// <summary>
/// スコアポップアップ情報の構造体
/// </summary>
struct ScorePopup
{
    int value;
    bool isHeadShot;
    float timer;
    float maxTimer;
};

/// <summary>
/// スコアUIクラス
/// </summary>
class ScoreUI : public UIBase
{
public:
    ScoreUI();
    virtual ~ScoreUI();

    void Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

    /// <summary>
    /// スコアポップアップを追加
    /// </summary>
    void AddScorePopup(int value, bool isHeadShot);

private:
    std::vector<ScorePopup> m_popups;
    int m_totalScorePopupValue;
    float m_totalScorePopupTimer;
    int m_scoreFont;

    static constexpr float kScorePopupDuration = 120.0f;
    static constexpr int kScorePopupX = 100;
    static constexpr int kScorePopupY = -50;

    // コンボアニメーション用
    int m_prevCombo = 0;
    float m_comboPulseScale = 1.0f;
    float m_comboAnimTimer = 0.0f;
};
