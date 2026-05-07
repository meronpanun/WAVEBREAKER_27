#include "SceneResult.h"
#include "EffekseerForDXLib.h"
#include "InputManager.h"
#include "SceneMain.h"
#include "SceneTitle.h"
#include "ScoreManager.h"
#include "Game.h"
#include "SoundManager.h"
#include <cassert>

namespace
{
    constexpr int kButtonWidth = 220;    // ボタンの幅
    constexpr int kButtonHeight = 60;    // ボタンの高さ
    constexpr int kButtonSpacing = 40;   // ボタン間のスペース
    constexpr int kBgImageSize = 1024;   // 背景画像のサイズ
    constexpr float kScrollSpeed = 1.0f; // 背景のスクロール速度
}

SceneResult::SceneResult()
    : m_background("data/image/GameClearBackGrand.png")
    , m_gameClearImage("data/image/GameClear.png")
    , m_japaneseFont("HGPｺﾞｼｯｸE", 30, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_arialBlackFont("Arial Black", 48, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_arialBlackLargeFont("Arial Black", 96, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_japaneseLargeFont("HGPｺﾞｼｯｸE", 54, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_japaneseButtonFont("HGPｺﾞｼｯｸE", 36, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_scrollX(0.0f)
    , m_scrollY(0.0f)
    , m_score(0)
    , m_killCount(0)
    , m_headShotCount(0)
    , m_maxCombo(0)
    , m_rank('C')
    , m_frame(0)
    , m_scoreAnim(0)
    , m_killAnim(0)
    , m_headAnim(0)
    , m_rankAnimAlpha(0)
    , m_buttonAnimAlpha(0)
    , m_rankScale(2.0f)
    , m_prevScale(1.0f)
{
    // スコア情報の取得
    m_score = ScoreManager::Instance().GetTotalScore();
    m_killCount = ScoreManager::Instance().GetBodyKillCount();
    m_headShotCount = ScoreManager::Instance().GetHeadKillCount();
    m_maxCombo = ScoreManager::Instance().GetMaxCombo();

    // ランク計算
    if (m_score >= 10000) m_rank = 'S';
    else if (m_score >= 5000) m_rank = 'A';
    else if (m_score >= 3000) m_rank = 'B';
    else m_rank = 'C';

    // Fonts setup
    ReloadFonts(Game::GetUIScale());
}

// フォントリロード
void SceneResult::ReloadFonts(float scale)
{
    m_japaneseFont.Reload(scale);
    m_arialBlackFont.Reload(scale);
    m_arialBlackLargeFont.Reload(scale);
    m_japaneseLargeFont.Reload(scale);
    m_japaneseButtonFont.Reload(scale);
}

SceneResult::~SceneResult()
{
    // 自動解放されるため処理不要
}

void SceneResult::Init()
{
    // マウスカーソルの表示/非表示を設定
    SetMouseDispFlag(true);
    // スコア保存
    ScoreManager::Instance().SaveScore(ScoreManager::Instance().GetTotalScore());

    // カウントアップ演出用初期化
    ScoreManager::Instance().ResetDisplayValues();
    ScoreManager::Instance().SetTargetDisplayValues(
        ScoreManager::Instance().GetScore(),
        ScoreManager::Instance().GetTotalScore(),
        ScoreManager::Instance().GetBodyKillCount(),
        ScoreManager::Instance().GetHeadKillCount());

    // BGM再生
    SoundManager::GetInstance()->PlayBGM("BGM", "Result");
}

SceneBase* SceneResult::Update()
{
    UpdateLayout();

    // 背景をスクロール
    m_scrollX += kScrollSpeed;
    m_scrollY += kScrollSpeed;
    if (m_scrollX > kBgImageSize) m_scrollX -= kBgImageSize;
    if (m_scrollY > kBgImageSize) m_scrollY -= kBgImageSize;

    // スコア演出用の更新
    ScoreManager::Instance().Update();

    if (InputManager::GetInstance()->IsTriggerMouseLeft())
    {
        Vec2 mousePos = InputManager::GetInstance()->GetMousePos();

        if (mousePos.x >= m_layout.titleBtnX1 && mousePos.x <= m_layout.titleBtnX2 &&
            mousePos.y >= m_layout.titleBtnY1 && mousePos.y <= m_layout.titleBtnY2)
        {
            // BGMを停止
            SoundManager::GetInstance()->StopBGM();
            SoundManager::GetInstance()->Play("UI", "Return");

            // スコアをリセット
            ScoreManager::Instance().ResetAll();

            return new SceneTitle(true);
        }
        if (mousePos.x >= m_layout.retryBtnX1 && mousePos.x <= m_layout.retryBtnX2 &&
            mousePos.y >= m_layout.retryBtnY1 && mousePos.y <= m_layout.retryBtnY2)
        {
            // BGMを停止
            SoundManager::GetInstance()->StopBGM();
            SoundManager::GetInstance()->Play("UI", "Return");

            // スコアをリセット
            ScoreManager::Instance().ResetAll();

            return new SceneMain(true);
        }
    }
    return nullptr;
}

void SceneResult::Draw()
{
    // 背景を描画
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    // スクロール位置を画像サイズで割った余りを計算
    int offsetX = (int)m_scrollX % kBgImageSize;
    int offsetY = (int)m_scrollY % kBgImageSize;

    // 負の値になった場合、正の値に補正
    if (offsetX < 0) offsetX += kBgImageSize;
    if (offsetY < 0) offsetY += kBgImageSize;

    // 2x2のタイル状に背景を描画（画面全体を覆うように）
    for (int y = -1; y < 2; y++)
    {
        for (int x = -1; x < 2; x++)
        {
            int drawX = x * kBgImageSize + offsetX;
            int drawY = y * kBgImageSize + offsetY;
            DrawExtendGraph(drawX, drawY, drawX + kBgImageSize, drawY + kBgImageSize,
                m_background, true);
        }
    }

    // 全体への黒半透明オーバーレイで文字を読みやすくする
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
    DrawBox(0, 0, screenW, screenH, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // ゲームクリア画像を描画 (サイズと位置を調整)
    DrawExtendGraph(m_layout.imageDrawX, m_layout.imageDrawY,
        m_layout.imageDrawX + m_layout.imageDrawWidth,
        m_layout.imageDrawY + m_layout.imageDrawHeight,
        m_gameClearImage, true);

    // リザルト表示エリアの背景（グラデーション）
    // 上: 透明度のある濃い黒, 下: 少し明るい黒
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    DrawGradientBox(m_layout.resBgX, m_layout.resBgY, m_layout.resBgX + m_layout.resBgW, m_layout.resBgY + m_layout.resBgH, 0x000000, 0x404040);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 枠線 (二重線でリッチに)
    DrawBox(m_layout.resBgX, m_layout.resBgY, m_layout.resBgX + m_layout.resBgW, m_layout.resBgY + m_layout.resBgH, 0xffffff, false);
    DrawBox(m_layout.resBgX + 4, m_layout.resBgY + 4, m_layout.resBgX + m_layout.resBgW - 4, m_layout.resBgY + m_layout.resBgH - 4, 0xaaaaaa, false);

    int y = m_layout.textBaseY;
    int shadowOffset = 2; // 影のオフセット

    // スコア、キル数、タイムの表示 (左寄せ気味に揃える)
    // 合計スコア
    // 影
    DrawFormatStringToHandle(m_layout.textLabelX + shadowOffset, y + shadowOffset, 0x000000, m_japaneseLargeFont, "合計スコア");
    DrawFormatStringToHandle(m_layout.textValueX + shadowOffset, y + shadowOffset, 0x000000, m_japaneseLargeFont, "%d", ScoreManager::Instance().GetDisplayTotalScore());
    // 本体
    DrawFormatStringToHandle(m_layout.textLabelX, y, 0xffffff, m_japaneseLargeFont, "合計スコア");
    DrawFormatStringToHandle(m_layout.textValueX, y, 0xffffff, m_japaneseLargeFont, "%d", ScoreManager::Instance().GetDisplayTotalScore());
    y += m_layout.textIntervalHigh;

    // 倒した敵の数
    int killCount = ScoreManager::Instance().GetBodyKillCount() + ScoreManager::Instance().GetHeadKillCount();
    // 影
    DrawFormatStringToHandle(m_layout.textLabelX + shadowOffset, y + shadowOffset, 0x000000, m_japaneseLargeFont, "倒した敵の数");
    DrawFormatStringToHandle(m_layout.textValueX + shadowOffset, y + shadowOffset, 0x000000, m_japaneseLargeFont, "%d", killCount);
    // 本体
    DrawFormatStringToHandle(m_layout.textLabelX, y, 0xffffff, m_japaneseLargeFont, "倒した敵の数");
    DrawFormatStringToHandle(m_layout.textValueX, y, 0xffffff, m_japaneseLargeFont, "%d", killCount);
    y += m_layout.textIntervalHigh;

    // クリアタイム
    // 影
    DrawFormatStringToHandle(m_layout.textLabelX + shadowOffset, y + shadowOffset, 0x000000, m_japaneseLargeFont, "クリアタイム");
    DrawFormatStringToHandle(m_layout.textValueX + shadowOffset, y + shadowOffset, 0x000000, m_japaneseLargeFont, "%.1f秒", SceneMain::GetElapsedTime());
    // 本体
    DrawFormatStringToHandle(m_layout.textLabelX, y, 0xffffff, m_japaneseLargeFont, "クリアタイム");
    DrawFormatStringToHandle(m_layout.textValueX, y, 0xffffff, m_japaneseLargeFont, "%.1f秒", SceneMain::GetElapsedTime());

    // ハイスコア表示
    int highScoreY = m_layout.highScoreY;
    float scale = Game::GetUIScale();
    int highScoreInterval = (int)(60 * scale);

    // タイトル
    int highScoreTextWidth = GetDrawStringWidthToHandle("--- High Score ---", -1, m_arialBlackFont);
    // 影
    DrawFormatStringToHandle(screenW * 0.5f - highScoreTextWidth * 0.5f + shadowOffset, highScoreY + shadowOffset, 0x000000, m_arialBlackFont, "--- High Score ---");
    // 本体 (金色のグラデーションっぽく見せるため、黄色で点滅させてもいいが、今回は固定)
    DrawFormatStringToHandle(screenW * 0.5f - highScoreTextWidth * 0.5f, highScoreY, 0xffff00, m_arialBlackFont, "--- High Score ---");
    highScoreY += highScoreInterval;

    const auto& scores = ScoreManager::Instance().GetHighScores();
    for (int i = 0; i < 3 && i < (int)scores.size(); ++i)
    {
        char highStr[64];
        sprintf_s(highStr, sizeof(highStr), "%d位: %d", i + 1, scores[i]);
        int highStrWidth = GetDrawStringWidthToHandle(highStr, -1, m_japaneseLargeFont);
        unsigned int color = 0xffffff;
        if (i == 0) color = 0xffd700;
        else if (i == 1) color = 0xc0c0c0; 
        else if (i == 2) color = 0xff8c00; 

        // 影
        DrawFormatStringToHandle(screenW * 0.5f - highStrWidth * 0.5f + shadowOffset, highScoreY + shadowOffset, 0x000000, m_japaneseLargeFont, "%s", highStr);
        // 本体
        DrawFormatStringToHandle(screenW * 0.5f - highStrWidth * 0.5f, highScoreY, color, m_japaneseLargeFont, "%s", highStr);
        highScoreY += highScoreInterval;
    }

    // ボタン描画
    Vec2 mousePos = InputManager::GetInstance()->GetMousePos();

    auto DrawButton = [&](int x1, int y1, int x2, int y2, const char* text, bool isHover)
    {
        // グラデーション背景
        unsigned int topColor = isHover ? 0x666666 : 0x444444;
        unsigned int btmColor = isHover ? 0x444444 : 0x222222;
        DrawGradientBox(x1, y1, x2, y2, topColor, btmColor);

        // 枠線
        unsigned int borderColor = isHover ? 0xffffff : 0xaaaaaa;
        DrawBox(x1, y1, x2, y2, borderColor, false);
        if (isHover)
        {
            // ホバー時は内側にも枠線を引いて強調
            DrawBox(x1 + 2, y1 + 2, x2 - 2, y2 - 2, 0xffff00, false);
        }

        // テキスト
        int textWidth = GetDrawStringWidthToHandle(text, -1, m_japaneseButtonFont);
        int textHeight = (int)(36 * Game::GetUIScale());
        int textX = x1 + (x2 - x1 - textWidth) * 0.5f;
        int textY = y1 + (y2 - y1 - textHeight) * 0.5f;

        // 影
        DrawFormatStringToHandle(textX + 2, textY + 2, 0x000000, m_japaneseButtonFont, text);
        // 本体
        DrawFormatStringToHandle(textX, textY, 0xffffff, m_japaneseButtonFont, text);
    };

    // タイトルボタン
    bool isTitleHover = (mousePos.x >= m_layout.titleBtnX1 && mousePos.x <= m_layout.titleBtnX2 &&
        mousePos.y >= m_layout.titleBtnY1 && mousePos.y <= m_layout.titleBtnY2);
    DrawButton(m_layout.titleBtnX1, m_layout.titleBtnY1, m_layout.titleBtnX2, m_layout.titleBtnY2, "タイトルに戻る", isTitleHover);

    // リトライボタン
    bool isRetryHover = (mousePos.x >= m_layout.retryBtnX1 && mousePos.x <= m_layout.retryBtnX2 &&
        mousePos.y >= m_layout.retryBtnY1 && mousePos.y <= m_layout.retryBtnY2);
    DrawButton(m_layout.retryBtnX1, m_layout.retryBtnY1, m_layout.retryBtnX2, m_layout.retryBtnY2, "リトライ", isRetryHover);
}

void SceneResult::UpdateLayout()
{
    int screenW = Game::GetScreenWidth();
    int screenH = Game::GetScreenHeight();
    float scale = Game::GetUIScale();

    // スケール変更検知
    if (fabsf(scale - m_prevScale) > 0.001f)
    {
        ReloadFonts(scale);
        m_prevScale = scale;
    }

    // Draw Game Clear Image
    m_layout.imageDrawWidth = (int)(800 * scale);
    m_layout.imageDrawHeight = (int)(200 * scale);
    m_layout.imageDrawX = (screenW - m_layout.imageDrawWidth) * 0.5f;
    m_layout.imageDrawY = (int)(50 * scale);

    // Result BG
    m_layout.resBgW = (int)(700 * scale);
    m_layout.resBgH = (int)(240 * scale);
    m_layout.resBgX = (screenW - m_layout.resBgW) * 0.5f;
    m_layout.resBgY = (int)(280 * scale);

    // Text
    m_layout.textLabelX = m_layout.resBgX + (int)(100 * scale);
    m_layout.textValueX = m_layout.resBgX + (int)(450 * scale);
    m_layout.textBaseY = m_layout.resBgY + (int)(25 * scale);
    m_layout.textIntervalHigh = (int)(65 * scale);

    // High Score
    m_layout.highScoreY = m_layout.textBaseY + (int)((70 + 70 + 100 + 20) * scale);

    // Buttons
    int bottomMargin = (int)(150 * scale);
    int btnY = screenH - bottomMargin;

    m_layout.btnW = (int)(270 * scale);
    m_layout.btnH = (int)(70 * scale);
    int btnSpacing = (int)(60 * scale);
    int centerX = screenW * 0.5f;

    // Title Button
    m_layout.titleBtnX1 = centerX - m_layout.btnW - btnSpacing * 0.5f;
    m_layout.titleBtnY1 = btnY;
    m_layout.titleBtnX2 = centerX - btnSpacing * 0.5f;
    m_layout.titleBtnY2 = btnY + m_layout.btnH;

    // Retry Button
    m_layout.retryBtnX1 = centerX + btnSpacing * 0.5f;
    m_layout.retryBtnY1 = btnY;
    m_layout.retryBtnX2 = centerX + m_layout.btnW + btnSpacing * 0.5f;
    m_layout.retryBtnY2 = btnY + m_layout.btnH;
}

void SceneResult::DrawGradientBox(int x1, int y1, int x2, int y2, unsigned int topColor, unsigned int bottomColor)
{
    VERTEX2D Vertex[6];
    float fx1 = static_cast<float>(x1);
    float fy1 = static_cast<float>(y1);
    float fx2 = static_cast<float>(x2);
    float fy2 = static_cast<float>(y2);

    // 0xRRGGBB 形式からRGBを抽出
    unsigned char topR = (topColor >> 16) & 0xFF;
    unsigned char topG = (topColor >> 8) & 0xFF;
    unsigned char topB = topColor & 0xFF;

    unsigned char btmR = (bottomColor >> 16) & 0xFF;
    unsigned char btmG = (bottomColor >> 8) & 0xFF;
    unsigned char btmB = bottomColor & 0xFF;

    // 左上
    Vertex[0].pos = VGet(fx1, fy1, 0.0f); Vertex[0].rhw = 1.0f; Vertex[0].u = 0.0f; Vertex[0].v = 0.0f; Vertex[0].dif = GetColorU8(topR, topG, topB, 255);
    // 右上
    Vertex[1].pos = VGet(fx2, fy1, 0.0f); Vertex[1].rhw = 1.0f; Vertex[1].u = 0.0f; Vertex[1].v = 0.0f; Vertex[1].dif = GetColorU8(topR, topG, topB, 255);
    // 左下
    Vertex[2].pos = VGet(fx1, fy2, 0.0f); Vertex[2].rhw = 1.0f; Vertex[2].u = 0.0f; Vertex[2].v = 0.0f; Vertex[2].dif = GetColorU8(btmR, btmG, btmB, 255);

    // 左下
    Vertex[3] = Vertex[2];
    // 右上
    Vertex[4] = Vertex[1];
    // 右下
    Vertex[5].pos = VGet(fx2, fy2, 0.0f); Vertex[5].rhw = 1.0f; Vertex[5].u = 0.0f; Vertex[5].v = 0.0f; Vertex[5].dif = GetColorU8(btmR, btmG, btmB, 255);

    DrawPolygon2D(Vertex, 2, DX_NONE_GRAPH, true);
}
