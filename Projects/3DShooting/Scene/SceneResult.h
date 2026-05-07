#pragma once
#include "SceneBase.h"
#include "ManagedFont.h"
#include "ManagedGraph.h"
#include "ManagedSound.h"
#include <vector>

class SceneResult : public SceneBase
{
public:
    SceneResult();
    virtual ~SceneResult();
    void Init() override;
    SceneBase* Update() override;
    void Draw() override;
    void ReloadFonts(float scale);

private:
    ManagedGraph m_background;
    ManagedGraph m_gameClearImage;

    ManagedFont m_japaneseFont;
    ManagedFont m_arialBlackFont;
    ManagedFont m_arialBlackLargeFont;
    ManagedFont m_japaneseLargeFont;
    ManagedFont m_japaneseButtonFont;

    void UpdateLayout();
    void DrawGradientBox(int x1, int y1, int x2, int y2, unsigned int topColor, unsigned int bottomColor);

    int m_score;
    int m_killCount;
    int m_headShotCount;
    int m_maxCombo;
    char m_rank;

    int m_frame;
    int m_scoreAnim;
    int m_killAnim;
    int m_headAnim;
    int m_rankAnimAlpha;
    int m_buttonAnimAlpha;
    float m_rankScale;

    // 前回のスケール値
    float m_prevScale;

    struct ResultLayout
    {
        int titleY;
        int scoreLabelY;
        int scoreValueY;
        int killLabelY;
        int killValueY;
        int headLabelY;
        int headValueY;
        int rankY;
        int buttonY;

        int imageDrawWidth;
        int imageDrawHeight;
        int imageDrawX;
        int imageDrawY;

        int resBgW;
        int resBgH;
        int resBgX;
        int resBgY;

        int textLabelX;
        int textValueX;
        int textBaseY;
        int textIntervalHigh;
        int highScoreY;

        int btnW;
        int btnH;

        int titleBtnX1;
        int titleBtnY1;
        int titleBtnX2;
        int titleBtnY2;

        int retryBtnX1;
        int retryBtnY1;
        int retryBtnX2;
        int retryBtnY2;
    };
    ResultLayout m_layout;

    float m_scrollX;
    float m_scrollY;
};

