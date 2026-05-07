#include "TutorialManager.h"
#include "EffekseerForDXLib.h"
#include "InputManager.h"
#include "SceneMain.h"
#include "Game.h"
#include "WaveManager.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <sstream>

namespace
{
    // 時間関連
    constexpr float kFrameTime = 1.0f / 60.0f;    // 1フレームの時間
    constexpr float kCompleteWaitTime = 2.0f;     // チュートリアル完了後の待機時間
    constexpr float kStepCompleteWaitTime = 1.5f; // ステップ完了後の待機時間
    constexpr float kMoveAccumGoalTime = 2.0f; // 移動チュートリアルの目標累積時間
    constexpr float kViewAccumGoalTime = 1.0f; // 視点チュートリアルの目標累積時間
    constexpr float kJumpAccumGoalTime = 0.2f; // ジャンプチュートリアルの目標累積時間
    constexpr float kRunAccumGoalTime = 1.0f;  // 走行チュートリアルの目標累積時間
    constexpr float kCheckAnimDuration = 0.3f; // チェックマークのアニメーション時間

    // UI関連
    constexpr int kFontSize = 34;        // チュートリアルメッセージのフォントサイズ
    constexpr int kDefaultFontSize = 24; // デフォルトのフォントサイズ
    constexpr int kMessageOffsetX = 630; // メッセージのXオフセット
    constexpr int kInitialYPos = 60;     // メッセージの初期Y座標
    constexpr int kLineSpacing = 60;     // メッセージの行間
    constexpr int kCheckMarkBaseSize = 60;     // チェックマークの基本サイズ
    constexpr int kCheckMarkOffsetXMove = 420; // 移動チェックマークのXオフセット
    constexpr int kCheckMarkOffsetXOthers = 510; // それ以外のチェックマークのXオフセット
    constexpr int kCheckMarkOffsetY = 30; // チェックマークのYオフセット
    constexpr float kCheckMarkAnimScale = 2.0f; // チェックマークアニメーションの最大スケール
    constexpr int kKeyImageSize = 60; // キー画像のサイズ
    constexpr int kKeyImageSpacing = 8;        // キー画像の間隔
    constexpr int kKeyImageWidth = 120;        // キー画像の幅
    constexpr int kKeyImageHeight = 60;        // キー画像の高さ
    constexpr int kShiftImageWidth = 120;      // Shiftキー画像の幅
    constexpr float kCheckMarkDrawSize = 1.0f; // チャックマーク画像の大きさ

    // UIボックス関連
    constexpr int kUIOffsetY = 200;              // UIのY座標オフセット
    constexpr int kBoxPaddingX = 30;             // ボックスの左右パディング
    constexpr int kBoxPaddingY = 15;             // ボックスの上下パディング
    constexpr int kBoxAlpha = 180;               // ボックスのアルファ値
    constexpr unsigned int kBoxColor = 0x000000; // ボックスの色

    // アニメーション関連
    constexpr float kUIAnimationSpeed = 15.0f;    // UIがスライドする速度
    constexpr float kUIOffscreenOffsetX = 750.0f; // UIの画面外オフセット

    // タイトル関連
    constexpr int kTitleFontSize = 42;    // タイトルのフォントサイズ
    constexpr int kTitleOffsetY = 15;     // タイトルのYオフセット
    constexpr int kTitleColor = 0xFFFFFF; // タイトルの色

    // マウスの移動量閾値
    constexpr float kMouseMovementThreshold = 2.0f;

    // メッセージ関連
    constexpr float kMessageDisplayTime = 5.0f; // メッセージの表示時間
    constexpr int kMessageTitleFontSize = 36;
    constexpr int kMessageDetailFontSize = 28;
    constexpr int kMessageOffsetY = 220;
    constexpr int kMessageLineSpacing = 8;      // メッセージの行間
    constexpr int kMessageTimeBarHeight = 6;    // タイムバーの高さ
    constexpr int kMessageTimeBarPaddingY = 24; // タイムバーの上下のパディング
}

TutorialManager::TutorialManager()
    : m_step(Step::None)
    , m_uiState(UIState::Hidden)
    , m_uiXOffset(kUIOffscreenOffsetX)
    , m_hasCompletedMove(false)
    , m_hasCompletedView(false)
    , m_checkMarkHandle("data/image/CheckMark.png")
    , m_prevMousePos{ 0, 0 }
    , m_moveAccumTime(0.0f)
    , m_viewAccumTime(0.0f)
    , m_completeWaitTime(0.0f)
    , m_isDisplayingCompletion(false)
    , m_isPlayingMoveCheckAnim(false)
    , m_moveCheckAnimTime(0.0f)
    , m_isPlayingViewCheckAnim(false)
    , m_viewCheckAnimTime(0.0f)
    , m_hasCompletedJump(false)
    , m_hasCompletedRun(false)
    , m_jumpAccumTime(0.0f)
    , m_runAccumTime(0.0f)
    , m_isPlayingJumpCheckAnim(false)
    , m_jumpCheckAnimTime(0.0f)
    , m_isPlayingRunCheckAnim(false)
    , m_runCheckAnimTime(0.0f)
    , m_stepCompleteWaitTime(0.0f)
    , m_isStepCompleted(false)
    , m_wKeyHandle("data/image/W.png")
    , m_aKeyHandle("data/image/A.png")
    , m_sKeyHandle("data/image/S.png")
    , m_dKeyHandle("data/image/D.png")
    , m_mouseMoveHorHandle("data/image/MouseMoveHor.png")
    , m_spaceKeyHandle("data/image/Space.png")
    , m_leftShiftKeyHandle("data/image/LeftShift.png")
    , m_crossHandle("data/image/Cross.png")
    , m_japaneseFontHandle("HGPｺﾞｼｯｸE", 30, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_japaneseLargeFontHandle("HGPｺﾞｼｯｸE", 54, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_messageDetailFontHandle("HGPｺﾞｼｯｸE", kMessageDetailFontSize, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_prevScale(1.0f)
{
    // フォントの作成 (初期化リストで行われているためReloadFontsは不要だが、スケール適用のため呼ぶなら呼ぶ)
    ReloadFonts(1.0f);
}

TutorialManager::~TutorialManager()
{
    // 自動解放されるため処理不要
}

void TutorialManager::ReloadFonts(float scale)
{
    m_japaneseFontHandle.Reload(scale);
    m_japaneseLargeFontHandle.Reload(scale);
    m_messageDetailFontHandle.Reload(scale);
}

void TutorialManager::Init()
{
    m_step = Step::Move;
    m_uiState = UIState::Hidden; // 最初は隠しておく
}

void TutorialManager::UpdateUI()
{
    switch (m_uiState)
    {
    case UIState::Hidden:

        // 新しいステップが始まったらEntering状態へ
        if (m_step != Step::None && m_step != Step::Completed)
        {
            m_uiState = UIState::Entering;
        }
        break;
    case UIState::Entering:
        m_uiXOffset -= kUIAnimationSpeed;
        if (m_uiXOffset <= 0.0f)
        {
            m_uiXOffset = 0.0f;
            m_uiState = UIState::OnScreen;
        }
        break;
    case UIState::OnScreen:
        // ステップ完了を待つ
        break;
    case UIState::Exiting:
        m_uiXOffset += kUIAnimationSpeed;
        if (m_uiXOffset >= kUIOffscreenOffsetX)
        {
            m_uiXOffset = kUIOffscreenOffsetX;
            m_uiState = UIState::Hidden;

            // 最後のステップだったら完了演出へ
            if (m_step == Step::Run)
            {
                m_isDisplayingCompletion = true;
                m_completeWaitTime = 0.0f;
            }

            // 次のステップへ
            m_step = static_cast<Step>(static_cast<int>(m_step) + 1);
        }
        break;
    }
}

void TutorialManager::Update()
{
    UpdateUI();
    UpdateMessages();

    // チュートリアル完了後の待機演出
    if (m_isDisplayingCompletion)
    {
        m_completeWaitTime += kFrameTime;
        if (m_completeWaitTime >= kCompleteWaitTime)
        {
            m_isDisplayingCompletion = false;
            m_step = Step::Completed;
        }
        return; // 待機中は他の処理をしない
    }

    // アニメタイマーを進める
    if (m_isPlayingMoveCheckAnim) m_moveCheckAnimTime += kFrameTime;
    if (m_isPlayingViewCheckAnim) m_viewCheckAnimTime += kFrameTime;
    if (m_isPlayingJumpCheckAnim) m_jumpCheckAnimTime += kFrameTime;
    if (m_isPlayingRunCheckAnim)  m_runCheckAnimTime  += kFrameTime;

    // UIが表示されているときだけ入力チェック
    if (m_uiState != UIState::OnScreen) return;

    // ステップ完了後の待機処理
    if (m_isStepCompleted)
    {
        m_stepCompleteWaitTime += kFrameTime;
        if (m_stepCompleteWaitTime >= kStepCompleteWaitTime)
        {
            m_isStepCompleted = false;
            m_uiState = UIState::Exiting; // 退場開始
        }
        return; // 待機中は他の入力を受け付けない
    }

    switch (m_step)
    {
    case Step::Move:
        if (!m_hasCompletedMove)
        {
            bool isMoving = CheckHitKey(KEY_INPUT_W) || CheckHitKey(KEY_INPUT_A) || CheckHitKey(KEY_INPUT_S) || CheckHitKey(KEY_INPUT_D);

            if (isMoving) m_moveAccumTime += kFrameTime;

            if (m_moveAccumTime >= kMoveAccumGoalTime)
            {
                m_hasCompletedMove = true;
                m_isPlayingMoveCheckAnim = true;
                m_moveCheckAnimTime = 0.0f;
            }
        }
        else if (m_moveCheckAnimTime >= kCheckAnimDuration) // チェックアニメ完了後
        {
            m_isStepCompleted = true; // 待機開始
            m_stepCompleteWaitTime = 0.0f;
        }
        break;
    case Step::View:
        if (!m_hasCompletedView)
        {
            Vec2 now = InputManager::GetInstance()->GetMousePos();
            float dx = now.x - m_prevMousePos.x;
            float dy = now.y - m_prevMousePos.y;

            if (std::abs(dx) > kMouseMovementThreshold || std::abs(dy) > kMouseMovementThreshold)
            {
                m_viewAccumTime += kFrameTime;
            }

            if (m_viewAccumTime >= kViewAccumGoalTime)
            {
                m_hasCompletedView = true;
                m_isPlayingViewCheckAnim = true;
                m_viewCheckAnimTime = 0.0f;
            }
            m_prevMousePos = now;
        }
        else if (m_viewCheckAnimTime >= kCheckAnimDuration)
        {
            m_isStepCompleted = true; // 待機開始
            m_stepCompleteWaitTime = 0.0f;
        }
        break;
    case Step::Jump:
        if (!m_hasCompletedJump)
        {
            if (CheckHitKey(KEY_INPUT_SPACE)) m_jumpAccumTime += kFrameTime;

            if (m_jumpAccumTime >= kJumpAccumGoalTime)
            {
                m_hasCompletedJump = true;
                m_isPlayingJumpCheckAnim = true;
                m_jumpCheckAnimTime = 0.0f;
            }
        }
        else if (m_jumpCheckAnimTime >= kCheckAnimDuration)
        {
            m_isStepCompleted = true; // 待機開始
            m_stepCompleteWaitTime = 0.0f;
        }
        break;
    case Step::Run:
        if (!m_hasCompletedRun)
        {
            if (CheckHitKey(KEY_INPUT_W) && CheckHitKey(KEY_INPUT_LSHIFT))
            {
                m_runAccumTime += kFrameTime;
            }

            if (m_runAccumTime >= kRunAccumGoalTime)
            {
                m_hasCompletedRun = true;
                m_isPlayingRunCheckAnim = true;
                m_runCheckAnimTime = 0.0f;
            }
        }
        else if (m_runCheckAnimTime >= kCheckAnimDuration)
        {
            m_isStepCompleted = true; // 待機開始
            m_stepCompleteWaitTime = 0.0f;
        }
        break;
    }
}

void TutorialManager::Draw(int screenW, int screenH)
{
    if (m_uiState == UIState::Hidden && m_messages.empty()) return;

    float scale = Game::GetUIScale();
    if (fabs(scale - m_prevScale) > 0.001f)
    {
        ReloadFonts(scale);
        m_prevScale = scale;
    }

    DrawMessages(screenW, screenH);

    const char* text = "";
    bool isDone = false;
    bool isCheckAnim = false;
    float checkAnimTime = 0.0f;

    switch (m_step)
    {
    case Step::Move:
    {
        isDone = m_hasCompletedMove;
        isCheckAnim = m_isPlayingMoveCheckAnim;
        checkAnimTime = m_moveCheckAnimTime;

        const char* remainingText = "で移動しよう!";
        int remainingTextWidth = GetDrawStringWidthToHandle(remainingText, strlen(remainingText), m_japaneseFontHandle);

        int scaledKeyImageSize = static_cast<int>(kKeyImageSize * scale);
        int scaledKeyImageSpacing = static_cast<int>(kKeyImageSpacing * scale);
        int scaledCheckMarkBaseSize = static_cast<int>(kCheckMarkBaseSize * scale);
        int scaledBoxPaddingX = static_cast<int>(kBoxPaddingX * scale);
        int scaledBoxPaddingY = static_cast<int>(kBoxPaddingY * scale);

        int imagesWidth = scaledKeyImageSize * 4 + scaledKeyImageSpacing * 3;
        int boxWidth = imagesWidth + remainingTextWidth + scaledCheckMarkBaseSize + scaledBoxPaddingX * 2;
        int boxHeight = scaledKeyImageSize + scaledBoxPaddingY * 2;

        int boxX = screenW - boxWidth - static_cast<int>(60 * scale) + static_cast<int>(m_uiXOffset * scale);
        int boxY = static_cast<int>(kUIOffsetY * scale);

        // 半透明の背景ボックスを描画
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
        DrawBox(boxX, boxY, boxX + boxWidth, boxY + boxHeight, kBoxColor, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // キー画像を描画
        int imageX = boxX + scaledBoxPaddingX;
        int imageY = boxY + scaledBoxPaddingY;
        DrawExtendGraph(imageX, imageY, imageX + scaledKeyImageSize, imageY + scaledKeyImageSize, m_wKeyHandle, true);
        imageX += scaledKeyImageSize + scaledKeyImageSpacing;

        DrawExtendGraph(imageX, imageY, imageX + scaledKeyImageSize, imageY + scaledKeyImageSize, m_aKeyHandle, true);
        imageX += scaledKeyImageSize + scaledKeyImageSpacing;

        DrawExtendGraph(imageX, imageY, imageX + scaledKeyImageSize, imageY + scaledKeyImageSize, m_sKeyHandle, true);
        imageX += scaledKeyImageSize + scaledKeyImageSpacing;

        DrawExtendGraph(imageX, imageY, imageX + scaledKeyImageSize, imageY + scaledKeyImageSize, m_dKeyHandle, true);

        // 残りのテキストを描画
        int textX = imageX + scaledKeyImageSize + scaledKeyImageSpacing;
        int textY = boxY + (boxHeight - static_cast<int>(kFontSize * scale)) * 0.5f;
        DrawStringToHandle(textX, textY, remainingText, 0xffffff, m_japaneseFontHandle);

        // チェックマークを描画
        if (isDone && m_checkMarkHandle >= 0)
        {
            float animScale = 1.0f;
            if (isCheckAnim && checkAnimTime < kCheckAnimDuration)
            {
                float t = checkAnimTime / kCheckAnimDuration;
                animScale = kCheckMarkAnimScale - t;
                if (animScale < 1.0f) animScale = 1.0f;
            }

            int size = static_cast<int>(scaledCheckMarkBaseSize * animScale);
            int cx = textX + remainingTextWidth + scaledBoxPaddingX + (scaledCheckMarkBaseSize * 0.5f);
            int cy = boxY + boxHeight * 0.5f;
            DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
        }
    } break;
    case Step::View:
    {
        isDone = m_hasCompletedView;
        isCheckAnim = m_isPlayingViewCheckAnim;
        checkAnimTime = m_viewCheckAnimTime;

        const char* remainingText = "で視点を動かそう!";
        int remainingTextWidth = GetDrawStringWidthToHandle(remainingText, strlen(remainingText), m_japaneseFontHandle);

        int scaledKeyImageSize = static_cast<int>(kKeyImageSize * scale);
        int scaledCheckMarkBaseSize = static_cast<int>(kCheckMarkBaseSize * scale);
        int scaledBoxPaddingX = static_cast<int>(kBoxPaddingX * scale);
        int scaledBoxPaddingY = static_cast<int>(kBoxPaddingY * scale);

        int imagesWidth = scaledKeyImageSize;
        int boxWidth = imagesWidth + remainingTextWidth + scaledCheckMarkBaseSize + scaledBoxPaddingX * 2;
        int boxHeight = scaledKeyImageSize + scaledBoxPaddingY * 2;
        int boxX = screenW - boxWidth - static_cast<int>(60 * scale) + static_cast<int>(m_uiXOffset * scale);
        int boxY = static_cast<int>(kUIOffsetY * scale);

        // 半透明の背景ボックスを描画
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
        DrawBox(boxX, boxY, boxX + boxWidth, boxY + boxHeight, kBoxColor, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // キー画像を描画
        int imageX = boxX + scaledBoxPaddingX;
        int imageY = boxY + scaledBoxPaddingY;
        DrawExtendGraph(imageX, imageY, imageX + scaledKeyImageSize, imageY + scaledKeyImageSize, m_mouseMoveHorHandle, true);

        // 残りのテキストを描画
        int textX = imageX + scaledKeyImageSize + 5;
        int textY = boxY + (boxHeight - static_cast<int>(kFontSize * scale)) * 0.5f;
        DrawStringToHandle(textX, textY, remainingText, 0xffffff, m_japaneseFontHandle);

        // チェックマークを描画
        if (isDone && m_checkMarkHandle >= 0)
        {
            float animScale = 1.0f;
            if (isCheckAnim && checkAnimTime < kCheckAnimDuration)
            {
                float t = checkAnimTime / kCheckAnimDuration;
                animScale = kCheckMarkAnimScale - t;
                if (animScale < 1.0f) animScale = 1.0f;
            }

            int size = static_cast<int>(scaledCheckMarkBaseSize * animScale);
            int cx = textX + remainingTextWidth + scaledBoxPaddingX + (scaledCheckMarkBaseSize * 0.5f);
            int cy = boxY + boxHeight * 0.5f;
            DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
        }
    } break;
    case Step::Jump:
    {
        isDone = m_hasCompletedJump;
        isCheckAnim = m_isPlayingJumpCheckAnim;
        checkAnimTime = m_jumpCheckAnimTime;

        const char* remainingText = "でジャンプ!";
        int remainingTextWidth = GetDrawStringWidthToHandle(remainingText, strlen(remainingText), m_japaneseFontHandle);

        int scaledKeyImageWidth = static_cast<int>(kKeyImageWidth * scale);
        int scaledKeyImageHeight = static_cast<int>(kKeyImageHeight * scale);
        int scaledCheckMarkBaseSize = static_cast<int>(kCheckMarkBaseSize * scale);
        int scaledBoxPaddingX = static_cast<int>(kBoxPaddingX * scale);
        int scaledBoxPaddingY = static_cast<int>(kBoxPaddingY * scale);

        int imagesWidth = scaledKeyImageWidth;
        int boxWidth = imagesWidth + remainingTextWidth + scaledCheckMarkBaseSize + scaledBoxPaddingX * 2;
        int boxHeight = scaledKeyImageHeight + scaledBoxPaddingY * 2;

        int boxX = screenW - boxWidth - static_cast<int>(60 * scale) + static_cast<int>(m_uiXOffset * scale);
        int boxY = static_cast<int>(kUIOffsetY * scale);

        // 半透明の背景ボックスを描画
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
        DrawBox(boxX, boxY, boxX + boxWidth, boxY + boxHeight, kBoxColor, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // キー画像を描画
        int imageX = boxX + scaledBoxPaddingX;
        int imageY = boxY + scaledBoxPaddingY;
        DrawExtendGraph(imageX, imageY, imageX + scaledKeyImageWidth, imageY + scaledKeyImageHeight, m_spaceKeyHandle, true);

        // 残りのテキストを描画
        int textX = imageX + scaledKeyImageWidth + 5;
        int textY = boxY + (boxHeight - static_cast<int>(kFontSize * scale)) * 0.5f;
        DrawStringToHandle(textX, textY, remainingText, 0xffffff, m_japaneseFontHandle);

        // チェックマークを描画
        if (isDone && m_checkMarkHandle >= 0)
        {
            float animScale = 1.0f;
            if (isCheckAnim && checkAnimTime < kCheckAnimDuration)
            {
                float t = checkAnimTime / kCheckAnimDuration;
                animScale = kCheckMarkAnimScale - t;
                if (animScale < 1.0f) animScale = 1.0f;
            }

            int size = static_cast<int>(scaledCheckMarkBaseSize * animScale);
            int cx = textX + remainingTextWidth + scaledBoxPaddingX + (scaledCheckMarkBaseSize * 0.5f);
            int cy = boxY + boxHeight * 0.5f;
            DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
        }
    } break;
    case Step::Run:
    {
        isDone = m_hasCompletedRun;
        isCheckAnim = m_isPlayingRunCheckAnim;
        checkAnimTime = m_runCheckAnimTime;

        const char* remainingText = "で走ろう!";
        int remainingTextWidth = GetDrawStringWidthToHandle(remainingText, strlen(remainingText), m_japaneseFontHandle);

        int scaledKeyImageSize = static_cast<int>(kKeyImageSize * scale);
        int scaledKeyImageSpacing = static_cast<int>(kKeyImageSpacing * scale);
        int scaledShiftImageWidth = static_cast<int>(kShiftImageWidth * scale);
        int scaledCheckMarkBaseSize = static_cast<int>(kCheckMarkBaseSize * scale);
        int scaledBoxPaddingX = static_cast<int>(kBoxPaddingX * scale);
        int scaledBoxPaddingY = static_cast<int>(kBoxPaddingY * scale);

        int imagesWidth = scaledShiftImageWidth + scaledKeyImageSize + scaledKeyImageSize + scaledKeyImageSpacing * 2;
        int boxWidth = imagesWidth + remainingTextWidth + scaledCheckMarkBaseSize + scaledBoxPaddingX * 2;
        int boxHeight = static_cast<int>(kShiftImageWidth * 0.5f * scale) + scaledBoxPaddingY * 2;

        int boxX = screenW - boxWidth - static_cast<int>(60 * scale) + static_cast<int>(m_uiXOffset * scale);
        int boxY = static_cast<int>(kUIOffsetY * scale);

        // 半透明の背景ボックスを描画
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
        DrawBox(boxX, boxY, boxX + boxWidth, boxY + boxHeight, kBoxColor, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // キー画像を描画
        int imageX = boxX + scaledBoxPaddingX;
        int imageY = boxY + scaledBoxPaddingY;
        DrawExtendGraph(imageX, imageY, imageX + scaledShiftImageWidth, imageY + scaledKeyImageSize, m_leftShiftKeyHandle, true);
        imageX += scaledShiftImageWidth + scaledKeyImageSpacing;

        DrawExtendGraph(imageX, imageY, imageX + scaledKeyImageSize, imageY + scaledKeyImageSize, m_crossHandle, true);
        imageX += scaledKeyImageSize + scaledKeyImageSpacing;

        DrawExtendGraph(imageX, imageY, imageX + scaledKeyImageSize, imageY + scaledKeyImageSize, m_wKeyHandle, true);

        // 残りのテキストを描画
        int textX = imageX + scaledKeyImageSize + 5;
        int textY = boxY + (boxHeight - static_cast<int>(kFontSize * scale)) * 0.5f;
        DrawStringToHandle(textX, textY, remainingText, 0xffffff, m_japaneseFontHandle);

        // チェックマークを描画
        if (isDone && m_checkMarkHandle >= 0)
        {
            float animScale = kCheckMarkDrawSize;
            if (isCheckAnim && checkAnimTime < kCheckAnimDuration)
            {
                float t = checkAnimTime / kCheckAnimDuration;
                animScale = kCheckMarkAnimScale - t;
                if (animScale < 1.0f) animScale = 1.0f;
            }

            int size = static_cast<int>(scaledCheckMarkBaseSize * animScale);
            int cx = textX + remainingTextWidth + scaledBoxPaddingX + (scaledCheckMarkBaseSize * 0.5f);
            int cy = boxY + boxHeight * 0.5f;
            DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
        }
    } break;
    default:
        return;

        if (m_step != Step::Move && m_step != Step::View && m_step != Step::Jump && m_step != Step::Run)
        {
            int textWidth = GetDrawStringWidthToHandle(text, strlen(text), m_japaneseFontHandle);

            int scaledCheckMarkBaseSize = static_cast<int>(kCheckMarkBaseSize * scale);
            int scaledBoxPaddingX = static_cast<int>(kBoxPaddingX * scale);
            int scaledBoxPaddingY = static_cast<int>(kBoxPaddingY * scale);

            int boxWidth = textWidth + scaledCheckMarkBaseSize + scaledBoxPaddingX * 2;
            int boxHeight = scaledCheckMarkBaseSize + scaledBoxPaddingY * 2;
            int boxX = screenW - boxWidth - static_cast<int>(60 * scale) + static_cast<int>(m_uiXOffset * scale);
            int boxY = static_cast<int>(kUIOffsetY * scale);

            // 半透明の背景ボックスを描画
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
            DrawBox(boxX, boxY, boxX + boxWidth, boxY + boxHeight, kBoxColor, true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            // テキストを描画
            int textX = boxX + scaledBoxPaddingX;
            int textY = boxY + (boxHeight - static_cast<int>(kFontSize * scale)) * 0.5f;
            DrawStringToHandle(textX, textY, text, 0xffffff, m_japaneseFontHandle);

            // チェックマークを描画
            if (isDone && m_checkMarkHandle >= 0)
            {
                float animScale = 1.0f;
                if (isCheckAnim && checkAnimTime < kCheckAnimDuration)
                {
                    float t = checkAnimTime / kCheckAnimDuration;
                    animScale = kCheckMarkAnimScale - t;
                    if (animScale < 1.0f) animScale = 1.0f;
                }

                int size = static_cast<int>(scaledCheckMarkBaseSize * animScale);
                int cx = textX + textWidth + scaledBoxPaddingX + (scaledCheckMarkBaseSize * 0.5f);
                int cy = boxY + boxHeight * 0.5f;
                DrawExtendGraph(cx - size * 0.5f, cy - size * 0.5f, cx + size * 0.5f, cy + size * 0.5f, m_checkMarkHandle, true);
            }
        }
    }
}

// チュートリアルがアクティブかどうか
bool TutorialManager::IsActive() const
{
    return m_step != Step::None && m_step != Step::Completed;
}

// チュートリアルが完了したかどうか
bool TutorialManager::IsCompleted() const
{
    return m_step == Step::Completed;
}

// チュートリアルをスキップして完了済みにする
void TutorialManager::Skip()
{
    m_step = Step::Completed;
    m_uiState = UIState::Hidden;
    m_isDisplayingCompletion = false;
    m_hasCompletedMove = true;
    m_hasCompletedView = true;
    m_hasCompletedJump = true;
    m_hasCompletedRun = true;
}

void TutorialManager::AddMessage(const std::string& title,
    const std::string& detail)
{
    m_messages.push_back({ title, detail, kUIOffscreenOffsetX, 0.0f, UIState::Entering });
}

void TutorialManager::UpdateMessages()
{
    for (auto& msg : m_messages)
    {
        switch (msg.state)
        {
        case UIState::Entering:
            msg.xOffset -= kUIAnimationSpeed;
            if (msg.xOffset <= 0.0f)
            {
                msg.xOffset = 0.0f;
                msg.state = UIState::OnScreen;
                msg.displayTimer = 0.0f;
            }
            break;
        case UIState::OnScreen:
            msg.displayTimer += kFrameTime;
            if (msg.displayTimer >= kMessageDisplayTime)
            {
                msg.state = UIState::Exiting;
            }
            break;
        case UIState::Exiting:
            msg.xOffset += kUIAnimationSpeed;
            if (msg.xOffset >= kUIOffscreenOffsetX)
            {
                msg.state = UIState::Hidden;
            }
            break;
        case UIState::Hidden:
            break;
        }
    }

    // 隠れたメッセージを削除
    m_messages.erase(std::remove_if(m_messages.begin(), m_messages.end(),
        [](const TutorialMessage& msg)
        {
            return msg.state == UIState::Hidden;
        }),
        m_messages.end());
}

void TutorialManager::DrawMessages(int screenW, int screenH)
{
    float scale = Game::GetUIScale();
    int scaledBoxPaddingX = static_cast<int>(kBoxPaddingX * scale);
    int scaledBoxPaddingY = static_cast<int>(kBoxPaddingY * scale);
    int scaledMessageTitleFontSize = static_cast<int>(kMessageTitleFontSize * scale);
    int scaledMessageTimeBarPaddingY = static_cast<int>(kMessageTimeBarPaddingY * scale);
    int scaledMessageTimeBarHeight = static_cast<int>(kMessageTimeBarHeight * scale);
    int scaledMessageDetailFontSize = static_cast<int>(kMessageDetailFontSize * scale);
    int scaledMessageLineSpacing = static_cast<int>(kMessageLineSpacing * scale);

    int yPos = static_cast<int>(kMessageOffsetY * scale);
    for (const auto& msg : m_messages)
    {
        // detail文字列を'\n'で分割
        std::vector<std::string> detailLines;
        std::string currentLine;
        std::istringstream iss(msg.detail);
        while (std::getline(iss, currentLine, '\n'))
        {
            detailLines.push_back(currentLine);
        }

        // 幅の計算
        int titleWidth = GetDrawStringWidthToHandle(msg.title.c_str(), msg.title.length(), m_japaneseLargeFontHandle);
        int maxDetailWidth = 0;
        for (const auto& line : detailLines)
        {
            int lineWidth = GetDrawStringWidthToHandle(line.c_str(), line.length(), m_messageDetailFontHandle);
            if (lineWidth > maxDetailWidth)
            {
                maxDetailWidth = lineWidth;
            }
        }
        int boxWidth = (std::max)(titleWidth, maxDetailWidth) + scaledBoxPaddingX * 2;

        // 高さの計算
        int boxHeight = scaledBoxPaddingY;
        boxHeight += scaledMessageTitleFontSize;
        boxHeight += scaledMessageTimeBarPaddingY;
        boxHeight += scaledMessageTimeBarHeight;
        boxHeight += scaledMessageTimeBarPaddingY;
        boxHeight += scaledMessageDetailFontSize * detailLines.size();
        if (detailLines.size() > 1)
        {
            boxHeight += scaledMessageLineSpacing * (detailLines.size() - 1);
        }
        boxHeight += scaledBoxPaddingY;

        // 描画
        int boxX = screenW - boxWidth - static_cast<int>(60 * scale) + static_cast<int>(msg.xOffset * scale);
        int boxY = yPos;

        // 背景ボックス
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBoxAlpha);
        DrawBox(boxX, boxY, boxX + boxWidth, boxY + boxHeight, kBoxColor, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // テキストとタイムバーを上から順に描画
        int currentY = boxY + scaledBoxPaddingY;
        int contentX = boxX + scaledBoxPaddingX;
        int contentWidth = boxWidth - scaledBoxPaddingX * 2;

        // タイトル
        DrawStringToHandle(contentX, currentY, msg.title.c_str(), 0xFFFFFF, m_japaneseLargeFontHandle);
        currentY += scaledMessageTitleFontSize + scaledMessageTimeBarPaddingY;

        // タイムバー
        if (msg.state == UIState::OnScreen)
        {
            float progress = 1.0f - (msg.displayTimer / kMessageDisplayTime);
            int barWidth = static_cast<int>(contentWidth * progress);
            DrawBox(contentX, currentY, contentX + barWidth, currentY + scaledMessageTimeBarHeight, 0xFF0000, true);
        }
        currentY += scaledMessageTimeBarHeight + scaledMessageTimeBarPaddingY;

        // 詳細 (複数行、色分けあり)
        for (const auto& line : detailLines)
        {
            int currentX = contentX;

            std::string s = line;
            std::string keyword1 = "回復アイテム";
            std::string keyword2 = "積極的に行動せよ";
            size_t pos1 = s.find(keyword1);
            size_t pos2 = s.find(keyword2);

            if (pos1 != std::string::npos)
            {
                std::string part1 = s.substr(0, pos1);
                std::string part3 = s.substr(pos1 + keyword1.length());

                DrawStringToHandle(currentX, currentY, part1.c_str(), 0xFFFFFF, m_messageDetailFontHandle);
                currentX += GetDrawStringWidthToHandle(part1.c_str(), part1.length(), m_messageDetailFontHandle);

                DrawStringToHandle(currentX, currentY, keyword1.c_str(), 0xFFD700, m_messageDetailFontHandle);
                currentX += GetDrawStringWidthToHandle(keyword1.c_str(), keyword1.length(), m_messageDetailFontHandle);

                DrawStringToHandle(currentX, currentY, part3.c_str(), 0xFFFFFF, m_messageDetailFontHandle);
            }
            else if (pos2 != std::string::npos)
            {
                std::string part1 = s.substr(0, pos2);

                DrawStringToHandle(currentX, currentY, part1.c_str(), 0xFFFFFF, m_messageDetailFontHandle);
                currentX += GetDrawStringWidthToHandle(part1.c_str(), part1.length(), m_messageDetailFontHandle);

                DrawStringToHandle(currentX, currentY, keyword2.c_str(), 0xFFD700, m_messageDetailFontHandle);
            }
            else
            {
                DrawStringToHandle(contentX, currentY, line.c_str(), 0xFFFFFF, m_messageDetailFontHandle);
            }

            currentY += scaledMessageDetailFontSize + scaledMessageLineSpacing;
        }

        yPos += boxHeight + static_cast<int>(10 * scale);
    }
}
