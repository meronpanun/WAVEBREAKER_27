#include "SceneGameOver.h"
#include "EffekseerForDXLib.h"
#include "Game.h"
#include "InputManager.h"
#include "SceneMain.h"
#include "SceneTitle.h"
#include "SoundManager.h"
#include <cassert>

namespace SceneGameOverConstants
{
    constexpr int kButtonWidth = 220;       // ボタンの幅
    constexpr int kButtonHeight = 60;       // ボタンの高さ
    constexpr int kButtonSpacing = 40;      // ボタン間のスペース
    constexpr int kBgImageSize = 1024;      // 背景画像のサイズ
    constexpr float kScrollSpeed = 1.0f;    // 背景のスクロール速度
    constexpr int kImageChangeInterval = 3; // 画像切り替え間隔（フレーム数）

    // テキスト間隔
    constexpr float kTextInterval = 70.0f;
}

using namespace SceneGameOverConstants;

SceneGameOver::SceneGameOver(int wave, int killCount, int score)
    : m_wave(wave)
    , m_killCount(killCount)
    , m_score(score)
    , m_background("data/image/BackGrand.png")
    , m_gameOverImage("data/image/GameOverZombie.png")
    , m_gameOverImage2("data/image/GameOverZombie2.png")
    , m_gameOverImage3("data/image/GameOverZombie3.png")
    , m_japaneseFont("HGPｺﾞｼｯｸE", 20, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_arialBlackFont("Arial Black", 32, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_arialBlackLargeFont("Arial Black", 64, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_japaneseLargeFont("HGPｺﾞｼｯｸE", 36, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_japaneseButtonFont("HGPｺﾞｼｯｸE", 36, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_scrollX(0.0f)
    , m_scrollY(0.0f)
    , m_currentImageIndex(0)
    , m_imageChangeTimer(0)
    , m_imageChangeInterval(kImageChangeInterval)
{
    // フォントのリロード（初期化）
    ReloadFonts(Game::GetUIScale());
}

void SceneGameOver::ReloadFonts(float scale)
{
    m_japaneseFont.Reload(scale);
    m_arialBlackFont.Reload(scale);
    m_arialBlackLargeFont.Reload(scale);
    m_japaneseLargeFont.Reload(scale);
    m_japaneseButtonFont.Reload(scale);
}

SceneGameOver::~SceneGameOver()
{
    // 自動解放されるため処理不要
}

void SceneGameOver::Init()
{
    // マウスカーソルの表示/非表示を設定
    SetMouseDispFlag(true);

    // カウントアップ演出用スコア初期化
    ScoreManager::Instance().ResetDisplayScore();
    ScoreManager::Instance().SetTargetDisplayScore(ScoreManager::Instance().GetTotalScore());

    // BGM再生
    SoundManager::GetInstance()->PlayBGM("BGM", "GameOver");
}

SceneBase* SceneGameOver::Update()
{
    UpdateLayout();

    // 背景をスクロール
    m_scrollX += kScrollSpeed;
    m_scrollY += kScrollSpeed;
    if (m_scrollX > kBgImageSize) m_scrollX -= kBgImageSize;
    if (m_scrollY > kBgImageSize) m_scrollY -= kBgImageSize;

    // スコア演出用の更新
    ScoreManager::Instance().Update();

    // 画像切り替え演出の更新
    m_imageChangeTimer++;
    if (m_imageChangeTimer >= m_imageChangeInterval)
    {
        // ランダムで画像を選択（0:通常, 1:乱れ2, 2:乱れ3）
        m_currentImageIndex = rand() % 3;

        m_imageChangeTimer = 0;

        // より頻繁な切り替え
        m_imageChangeInterval = 5 + (rand() % 30);
    }

    if (InputManager::GetInstance()->IsTriggerMouseLeft())
    {
        Vec2 mousePos = InputManager::GetInstance()->GetMousePos();
        if (mousePos.x >= m_layout.titleBtnX1 &&
            mousePos.x <= m_layout.titleBtnX2 &&
            mousePos.y >= m_layout.titleBtnY1 &&
            mousePos.y <= m_layout.titleBtnY2)
        {
            // BGMを停止
            SoundManager::GetInstance()->StopBGM();
            SoundManager::GetInstance()->Play("UI", "Return");

            // スコアをリセット
            ScoreManager::Instance().ResetAll();

            return new SceneTitle(true);
        }
        if (mousePos.x >= m_layout.retryBtnX1 &&
            mousePos.x <= m_layout.retryBtnX2 &&
            mousePos.y >= m_layout.retryBtnY1 &&
            mousePos.y <= m_layout.retryBtnY2)
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

void SceneGameOver::Draw()
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
            DrawExtendGraph(drawX, drawY, drawX + kBgImageSize, drawY + kBgImageSize, m_background, true);
        }
    }

    // 全体への黒半透明オーバーレイで文字を読みやすくする
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
    DrawBox(0, 0, screenW, screenH, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 現在の画像インデックスに応じて画像を選択
    int currentImageHandle;
    switch (m_currentImageIndex)
    {
    case 0:
        currentImageHandle = m_gameOverImage;
        break;
    case 1:
        currentImageHandle = m_gameOverImage2;
        break;
    case 2:
        currentImageHandle = m_gameOverImage3;
        break;
    default:
        currentImageHandle = m_gameOverImage;
        break;
    }

    // 画像描画
    DrawExtendGraph(m_layout.imageDrawX, m_layout.imageDrawY,
                    m_layout.imageDrawX + m_layout.imageDrawWidth,
                    m_layout.imageDrawY + m_layout.imageDrawHeight,
                    currentImageHandle, true);

    // リザルト表示エリアの背景（グラデーション）
    // SceneResultと同様の配色
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    DrawGradientBox(m_layout.resBgX, m_layout.resBgY,
                    m_layout.resBgX + m_layout.resBgW,
                    m_layout.resBgY + m_layout.resBgH, 0x000000, 0x404040);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 枠線 (二重線でリッチに)
    DrawBox(m_layout.resBgX, m_layout.resBgY, m_layout.resBgX + m_layout.resBgW,
            m_layout.resBgY + m_layout.resBgH, 0xffffff, false);
    DrawBox(m_layout.resBgX + 4, m_layout.resBgY + 4,
            m_layout.resBgX + m_layout.resBgW - 4,
            m_layout.resBgY + m_layout.resBgH - 4, 0xaaaaaa, false);

    float scale = Game::GetUIScale();
    int textInterval = (int)(kTextInterval * scale);
    int textY = m_layout.textBaseY;
    int shadowOffset = 2; // 影のオフセット

    // テキスト配置
    char waveStr[64];
    sprintf_s(waveStr, sizeof(waveStr), "%dウェーブ生き残った", m_wave);
    // ウェーブ数は中央寄せ
    int waveStrW = GetDrawStringWidthToHandle(waveStr, strlen(waveStr), m_japaneseLargeFont);

    // 影
    DrawFormatStringToHandle((screenW - waveStrW) * 0.5f + shadowOffset, textY + shadowOffset, 0x000000, m_japaneseLargeFont, "%s", waveStr);

    // 本体
    DrawFormatStringToHandle((screenW - waveStrW) * 0.5f, textY, 0xffffff, m_japaneseLargeFont, "%s", waveStr);
    textY += textInterval;

    char killStr[64];
    int killCount = m_killCount;

    // 影
    DrawFormatStringToHandle(m_layout.textLabelX + shadowOffset, textY + shadowOffset, 0x000000, m_japaneseLargeFont, "倒した敵の数");
    DrawFormatStringToHandle(m_layout.textValueX + shadowOffset, textY + shadowOffset, 0x000000, m_japaneseLargeFont, "%d", killCount);

    // 本体
    DrawFormatStringToHandle(m_layout.textLabelX, textY, 0xffffff, m_japaneseLargeFont, "倒した敵の数");
    DrawFormatStringToHandle(m_layout.textValueX, textY, 0xffffff, m_japaneseLargeFont, "%d", killCount);
    textY += textInterval;

    char scoreStr[64];
    // 影
    DrawFormatStringToHandle(m_layout.textLabelX + shadowOffset, textY + shadowOffset, 0x000000, m_japaneseLargeFont, "スコア");
    DrawFormatStringToHandle(m_layout.textValueX + shadowOffset, textY + shadowOffset, 0x000000, m_japaneseLargeFont, "%d",
                             ScoreManager::Instance().GetDisplayScore());
    // 本体
    DrawFormatStringToHandle(m_layout.textLabelX, textY, 0xffffff, m_japaneseLargeFont, "スコア");
    DrawFormatStringToHandle(m_layout.textValueX, textY, 0xffffff, m_japaneseLargeFont, "%d",
                             ScoreManager::Instance().GetDisplayScore());

    // マウス位置取得
    Vec2 mousePos = InputManager::GetInstance()->GetMousePos();

    // ボタン描画ヘルパー
    auto DrawButton = [&](int x1, int y1, int x2, int y2, const char* text,
                          bool isHover) {
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
        int textHeight = (int)(36 * Game::GetUIScale()); // フォントサイズに合わせて調整
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

void SceneGameOver::DrawGradientBox(int x1, int y1, int x2, int y2, unsigned int topColor, unsigned int bottomColor)
{
    VERTEX2D Vertex[6];
    float fx1 = static_cast<float>(x1);
    float fy1 = static_cast<float>(y1);
    float fx2 = static_cast<float>(x2);
    float fy2 = static_cast<float>(y2);

    unsigned char topR = (topColor >> 16) & 0xFF;
    unsigned char topG = (topColor >> 8) & 0xFF;
    unsigned char topB = topColor & 0xFF;

    unsigned char btmR = (bottomColor >> 16) & 0xFF;
    unsigned char btmG = (bottomColor >> 8) & 0xFF;
    unsigned char btmB = bottomColor & 0xFF;

    // 左上
    Vertex[0].pos = VGet(fx1, fy1, 0.0f);
    Vertex[0].rhw = 1.0f;
    Vertex[0].u = 0.0f;
    Vertex[0].v = 0.0f;
    Vertex[0].dif = GetColorU8(topR, topG, topB, 255);
    // 右上
    Vertex[1].pos = VGet(fx2, fy1, 0.0f);
    Vertex[1].rhw = 1.0f;
    Vertex[1].u = 0.0f;
    Vertex[1].v = 0.0f;
    Vertex[1].dif = GetColorU8(topR, topG, topB, 255);
    // 左下
    Vertex[2].pos = VGet(fx1, fy2, 0.0f);
    Vertex[2].rhw = 1.0f;
    Vertex[2].u = 0.0f;
    Vertex[2].v = 0.0f;
    Vertex[2].dif = GetColorU8(btmR, btmG, btmB, 255);

    // 左下
    Vertex[3] = Vertex[2];
    // 右上
    Vertex[4] = Vertex[1];
    // 右下
    Vertex[5].pos = VGet(fx2, fy2, 0.0f);
    Vertex[5].rhw = 1.0f;
    Vertex[5].u = 0.0f;
    Vertex[5].v = 0.0f;
    Vertex[5].dif = GetColorU8(btmR, btmG, btmB, 255);

    DrawPolygon2D(Vertex, 2, DX_NONE_GRAPH, true);
}

void SceneGameOver::UpdateLayout()
{
    int screenW = Game::GetScreenWidth();
    int screenH = Game::GetScreenHeight();
    float scale = Game::GetUIScale();

    // 画像サイズの計算
    float imageAspect = 1024.0f / 1110.0f;
    float screenAspect = (float)screenW / (float)screenH;
    int drawWidth, drawHeight;
    if (imageAspect > screenAspect)
    {
        drawWidth = screenW;
        drawHeight = (int)(screenW / imageAspect);
    }
    else
    {
        drawHeight = screenH;
        drawWidth = (int)(screenH * imageAspect);
    }
    // 縮小スケール
    const float kScale = 0.4f;
    m_layout.imageDrawWidth = (int)(drawWidth * kScale);
    m_layout.imageDrawHeight = (int)(drawHeight * kScale);

    // 画面上部に配置
    const int kTopMargin = (int)(screenH * 0.04f);
    m_layout.imageDrawX = (screenW - m_layout.imageDrawWidth) * 0.5f;
    m_layout.imageDrawY = kTopMargin;

    // リザルト表示エリア
    m_layout.resBgW = (int)(700 * scale);
    m_layout.resBgH = (int)(260 * scale);
    m_layout.resBgX = (screenW - m_layout.resBgW) * 0.5f;
    m_layout.resBgY = m_layout.imageDrawY + m_layout.imageDrawHeight + (int)(20 * scale);

    // テキスト配置
    m_layout.textLabelX = m_layout.resBgX + (int)(100 * scale);
    m_layout.textValueX = m_layout.resBgX + (int)(450 * scale);
    m_layout.textBaseY = m_layout.resBgY + (int)(40 * scale);

    // ボタン
    m_layout.btnW = (int)(270 * scale);
    m_layout.btnH = (int)(70 * scale);
    int btnSpacing = (int)(60 * scale);
    int centerX = screenW * 0.5f;
    int btnBaseY = m_layout.resBgY + m_layout.resBgH + (int)(40 * scale);

    // タイトルに戻るボタン
    m_layout.titleBtnX1 = centerX - m_layout.btnW - btnSpacing * 0.5f;
    m_layout.titleBtnY1 = btnBaseY;
    m_layout.titleBtnX2 = centerX - btnSpacing * 0.5f;
    m_layout.titleBtnY2 = btnBaseY + m_layout.btnH;

    // リトライボタン
    m_layout.retryBtnX1 = centerX + btnSpacing * 0.5f;
    m_layout.retryBtnY1 = btnBaseY;
    m_layout.retryBtnX2 = centerX + m_layout.btnW + btnSpacing * 0.5f;
    m_layout.retryBtnY2 = btnBaseY + m_layout.btnH;
}
