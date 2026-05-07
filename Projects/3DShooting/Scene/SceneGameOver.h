#pragma once
#include "SceneBase.h"
#include "ManagedFont.h"
#include "ManagedGraph.h"
#include "ManagedSound.h"

/// <summary>
/// ゲームオーバーリザルトシーンクラス
/// </summary>
class SceneGameOver : public SceneBase
{
public:
    SceneGameOver(int wave, int killCount, int score);
    virtual ~SceneGameOver();

    void Init() override;
    SceneBase* Update() override;
    void Draw() override;

private:
    /// <summary>
    /// 画面サイズに合わせてレイアウトを計算する
    /// </summary>
    void UpdateLayout();

    /// <summary>
    /// グラデーションボックスを描画する
    /// </summary>
    void DrawGradientBox(int x1, int y1, int x2, int y2, unsigned int topColor, unsigned int bottomColor);

    void ReloadFonts(float scale);

private:
    // リソース管理
    ManagedGraph m_background;           // 背景画像のハンドル
    ManagedGraph m_gameOverImage;        // ゲームオーバー画像のハンドル
    ManagedGraph m_gameOverImage2;       // ゲームオーバー画像2（乱れた画像）
    ManagedGraph m_gameOverImage3;       // ゲームオーバー画像3（乱れた画像）
    ManagedFont m_japaneseFont;          // 日本語フォントハンドル
    ManagedFont m_arialBlackFont;        // Arial Blackフォントハンドル
    ManagedFont m_arialBlackLargeFont;   // Arial Blackラージフォントハンドル
    ManagedFont m_japaneseLargeFont;     // 日本語ラージフォントハンドル
    ManagedFont m_japaneseButtonFont;    // 日本語ボタンフォントハンドル

    // ゲーム結果情報
    int m_wave;      // ウェーブ
    int m_killCount; // キル数
    int m_score;     // スコア

    // 背景スクロール管理
    float m_scrollX; // 背景のスクロールX座標
    float m_scrollY; // 背景のスクロールY座標

    // 演出管理
    int m_currentImageIndex; // 現在表示中の画像インデックス
    int m_imageChangeTimer;    // 画像切り替えタイマー
    int m_imageChangeInterval; // 画像切り替え間隔（フレーム数）

    struct GameOverLayout
    {
        // 画像
        int imageDrawX, imageDrawY, imageDrawWidth, imageDrawHeight;

        // リザルト背景
        int resBgX, resBgY, resBgW, resBgH;

        // テキスト領域
        int textLabelX, textValueX, textBaseY;

        // ボタン
        int titleBtnX1, titleBtnY1, titleBtnX2, titleBtnY2;
        int retryBtnX1, retryBtnY1, retryBtnX2, retryBtnY2;
        int btnW, btnH;
    };

    GameOverLayout m_layout;
};
