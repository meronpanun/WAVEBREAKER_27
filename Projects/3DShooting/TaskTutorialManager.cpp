#include "TaskTutorialManager.h"
#include "DxLib.h"
#include "Game.h"
#include "Player.h"
#include "WaveManager.h"
#include "Scene/SceneMain.h"
#include "TutorialManager.h"
#include "TutorialTasks.h"
#include <string>
#include <algorithm>

namespace
{
    // UI関連
    constexpr int kTaskTextX = 60;
    constexpr int kTaskTextY = 200;
    constexpr int kTitleFontSize = 48;
    constexpr int kBarMaxWidth = 300;
    constexpr int kBarHeight = 22;

    // 背景ボックス関連
    constexpr int kBgBoxPaddingX = 20;
    constexpr int kBgBoxPaddingY = 15;
    constexpr int kBgBoxWidth = 580;
    constexpr int kBgBoxHeight = 250;
    constexpr unsigned int kBgBoxColor = 0x000000;
    constexpr int kBgBoxAlpha = 128;
}

// 静的メンバ変数の実体を定義
TaskTutorialManager* TaskTutorialManager::m_instance = nullptr;

TaskTutorialManager* TaskTutorialManager::GetInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new TaskTutorialManager();
    }
    return m_instance;
}

TaskTutorialManager::TaskTutorialManager()
    : m_pWaveManager(nullptr)
    , m_pPlayer(nullptr)
    , m_step(TaskStep::None)
    , m_currentTask(nullptr)
    , m_titleFont("HGPｺﾞｼｯｸE", 48, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_taskFont("HGPｺﾞｼｯｸE", 30, 3, DX_FONTTYPE_ANTIALIASING_EDGE_8X8)
    , m_diamondImg("data/image/Diamond.png")
    , m_mouseLeftImg("data/image/MouseLeft.png")
    , m_mouseRightImg("data/image/MouseRight.png")
    , m_alpha1Img("data/image/Alpha1.png")
    , m_alpha2Img("data/image/Alpha2.png")
    , m_mouseWheelImg("data/image/MouseWheel.png")
    , m_rKeyImg("data/image/R.png")
    , m_lockOnUIImg("data/image/LockOnUI.png")
    , m_mouseRightGuardImg("data/image/MouseRight.png")
    , m_designerImg("data/image/Designer.png")
    , m_titlePosX(0.0f)
    , m_titleAnimSpeed(15.0f)
    , m_isTitleAnimFinished(false)
    , m_taskAlpha(0)
    , m_taskFadeSpeed(15.0f) 
    , m_animationWaitTimer(0)
    , m_displayedProgress(0.0f)
    , m_progressAnimSpeed(0.05f)
    , m_transitionDelayTimer(0)
    , m_hasShownParryTutorial(false)
    , m_isParryTutorialPaused(false)
    , m_prevScale(1.0f)
{
    // フォントの作成
    ReloadFonts(1.0f);
}

TaskTutorialManager::~TaskTutorialManager()
{
}

void TaskTutorialManager::ReloadFonts(float scale)
{
    m_titleFont.Reload(scale);
    m_taskFont.Reload(scale);
}

void TaskTutorialManager::Init(WaveManager* pWaveManager, Player* pPlayer)
{
    m_pWaveManager = pWaveManager;
    m_pPlayer = pPlayer;
    m_step = TaskStep::Shoot;
    m_currentTask = std::make_unique<ShootTutorialTask>();
    m_currentTask->Start(m_pWaveManager, m_pPlayer);

    // アニメション初期化
    m_titlePosX = -450.0f;
    m_isTitleAnimFinished = false;
    m_taskAlpha = 0;
    m_animationWaitTimer = 0;
    m_displayedProgress = 0.0f;
    m_transitionDelayTimer = 0;
    m_hasShownParryTutorial = false;
    m_isParryTutorialPaused = false;
    
    m_restrictedActionTimer = 0;
    m_restrictedActionType = AttackType::None;
    m_restrictedActionAlpha = 0;

    if (m_pWaveManager)
    {
        m_pWaveManager->SpawnTutorialWave(1);
    }
    if (m_pPlayer)
    {
        m_pPlayer->SetAttackRestrictions(AttackType::Shoot);
    }
}

void TaskTutorialManager::NotifyEnemyKilled(AttackType attackType)
{
    if (m_currentTask) m_currentTask->NotifyEnemyKilled((int)attackType);
}

void TaskTutorialManager::NotifyShieldThrowKill()
{
    if (m_currentTask) m_currentTask->NotifyShieldThrowKill();
}

void TaskTutorialManager::NotifyRestrictedAction(AttackType attemptedType)
{
    // すでに表示中ならタイプだけ更新してタイマーリセット（または無視）
    // 違うタイプならリセット
    if (m_restrictedActionTimer > 0 && m_restrictedActionType == attemptedType)
    {
        // タイマー延長
        m_restrictedActionTimer = 120; 
    }
    else
    {
        m_restrictedActionType = attemptedType;
        m_restrictedActionTimer = 120;
        m_restrictedActionAlpha = 0; // フェードインから開始
    }
}

void TaskTutorialManager::NotifyParrySuccess()
{
    if (m_currentTask) m_currentTask->NotifyParrySuccess();
}

void TaskTutorialManager::NotifyParryableAttack()
{
    // パリィタスク中かつ、まだ説明を表示していない場合
    if (m_step == TaskStep::Parry && !m_hasShownParryTutorial && !m_isParryTutorialPaused)
    {
        m_isParryTutorialPaused = true;
        m_hasShownParryTutorial = true;
        Game::SetPaused(true); // ゲームを一時停止
    }
}

void TaskTutorialManager::Reset()
{
    m_step = TaskStep::None;
    m_currentTask = nullptr;
    m_pWaveManager = nullptr;
    if (m_pPlayer) m_pPlayer->SetAttackRestrictions(AttackType::None);
    m_pPlayer = nullptr;
    m_hasShownParryTutorial = false;
    m_isParryTutorialPaused = false;
    Game::SetPaused(false);
}

void TaskTutorialManager::Skip(WaveManager* pWaveManager)
{
    m_step = TaskStep::Completed;
    m_pWaveManager = pWaveManager;
    if (m_pPlayer) m_pPlayer->SetAttackRestrictions(AttackType::None);
    m_hasShownParryTutorial = false;
    m_isParryTutorialPaused = false;
    Game::SetPaused(false);
}

void TaskTutorialManager::SkipToParry()
{
    // 必要なポインタがセットされていない場合はSceneMainから取得を試みる
    if (!m_pWaveManager || !m_pPlayer)
    {
        if (SceneMain::Instance())
        {
            m_pWaveManager = SceneMain::Instance()->GetWaveManager();
            m_pPlayer = SceneMain::Instance()->GetPlayerPtr();
        }
    }

    // それでも取得できなければ無視
    if (!m_pWaveManager || !m_pPlayer) return;

    // 前半の基本チュートリアル(TutorialManager)が終わっていない状態で呼ばれた場合、
    // SceneMain側にTaskTutorialInitフラグを立てさせるか、強制的に切り替えるために
    // 基本操作チュートリアルを終わらせる必要がある。
    if (SceneMain::Instance() && SceneMain::Instance()->GetTutorialManager())
    {
        SceneMain::Instance()->GetTutorialManager()->Skip();
        SceneMain::Instance()->SetTaskTutorialInit(true); // SceneMain側でInit()の二重呼び出しを防ぐ
    }

    // パリィタスクから再開するように状態を上書き
    m_step = TaskStep::Parry;
    m_currentTask = std::make_unique<ParryTutorialTask>();
    m_currentTask->Start(m_pWaveManager, m_pPlayer);

    // アニメーションを最初から再生するように各種フラグ・座標をリセット
    m_titlePosX = -450.0f;
    m_isTitleAnimFinished = false;
    m_taskAlpha = 0;
    m_animationWaitTimer = 0;
    m_transitionDelayTimer = 0;
    m_displayedProgress = 0.0f;

    Game::SetPaused(false);
}



void TaskTutorialManager::Update()
{
    // パリィ説明表示中の更新処理
    if (m_isParryTutorialPaused)
    {
        if (GetMouseInput() & MOUSE_INPUT_RIGHT)
        {
            m_isParryTutorialPaused = false;
            Game::SetPaused(false);
        }
        return;
    }

    // 制限アクションフィードバックの更新
    if (m_restrictedActionTimer > 0)
    {
        m_restrictedActionTimer--;
        if (m_restrictedActionTimer > 100) m_restrictedActionAlpha = (std::min)(m_restrictedActionAlpha + 25, 255);
        else if (m_restrictedActionTimer < 30) m_restrictedActionAlpha = (std::max)(m_restrictedActionAlpha - 10, 0);
        else m_restrictedActionAlpha = 255;
    }

    // プログレスバーのアニメーション
    if (m_currentTask)
    {
        float target = m_currentTask->GetProgress();
        if (m_displayedProgress < target)
        {
            m_displayedProgress += m_progressAnimSpeed;
            if (m_displayedProgress > target) m_displayedProgress = target;
        }
    }

    // タイトルアニメーション
    if (!m_isTitleAnimFinished)
    {
        m_titlePosX += m_titleAnimSpeed;
        if (m_titlePosX >= kTaskTextX)
        {
            m_titlePosX = kTaskTextX;
            m_isTitleAnimFinished = true;
            m_animationWaitTimer = 30;
        }
    }
    else if (m_animationWaitTimer > 0)
    {
        m_animationWaitTimer--;
    }
    else if (m_taskAlpha < 255)
    {
        m_taskAlpha = (std::min)(m_taskAlpha + (int)m_taskFadeSpeed, 255);
    }

    // ステップ遷移ロジック
    switch (m_step)
    {
    case TaskStep::Shoot:
        if (m_currentTask && m_currentTask->IsCompleted())
        {
            m_step = TaskStep::ShootCompleteDelay;
            m_transitionDelayTimer = 60;
        }
        break;
    case TaskStep::ShootCompleteDelay:
        if (--m_transitionDelayTimer <= 0)
        {
            m_step = TaskStep::Tackle;
            m_currentTask = std::make_unique<TackleTutorialTask>();
            m_currentTask->Start(m_pWaveManager, m_pPlayer);
            m_titlePosX = -450.0f;
            m_isTitleAnimFinished = false;
            m_taskAlpha = 0;
            m_displayedProgress = 0.0f;
        }
        break;
    case TaskStep::Tackle:
        if (m_currentTask && m_currentTask->IsCompleted())
        {
            m_step = TaskStep::TackleCompleteDelay;
            m_transitionDelayTimer = 60;
        }
        break;
    case TaskStep::TackleCompleteDelay:
        if (--m_transitionDelayTimer <= 0)
        {
            m_step = TaskStep::ShieldThrow;
            m_currentTask = std::make_unique<ShieldThrowTutorialTask>();
            m_currentTask->Start(m_pWaveManager, m_pPlayer);
            m_titlePosX = -300.0f;
            m_isTitleAnimFinished = false;
            m_taskAlpha = 0;
            m_displayedProgress = 0.0f;
        }
        break;
    case TaskStep::ShieldThrow:
        if (m_currentTask && m_currentTask->IsCompleted())
        {
            m_step = TaskStep::ShieldThrowCompleteDelay;
            m_transitionDelayTimer = 60;
        }
        break;
    case TaskStep::ShieldThrowCompleteDelay:
        if (--m_transitionDelayTimer <= 0)
        {
            m_step = TaskStep::Parry;
            m_currentTask = std::make_unique<ParryTutorialTask>();
            m_currentTask->Start(m_pWaveManager, m_pPlayer);
            m_titlePosX = -300.0f;
            m_isTitleAnimFinished = false;
            m_taskAlpha = 0;
            m_displayedProgress = 0.0f;
        }
        break;
    case TaskStep::Parry:
        if (m_currentTask && m_currentTask->IsCompleted())
        {
            m_step = TaskStep::ParryCompleteDelay;
            m_transitionDelayTimer = 60;
        }
        break;
    case TaskStep::ParryCompleteDelay:
        if (--m_transitionDelayTimer <= 0)
        {
            m_step = TaskStep::Completed;
            if (m_pPlayer) m_pPlayer->SetAttackRestrictions(AttackType::None);
        }
        break;
    }
}

bool TaskTutorialManager::IsCompleted() const
{
    return m_step == TaskStep::Completed;
}

void TaskTutorialManager::Draw()
{
    float scale = Game::GetUIScale();
    if (fabs(scale - m_prevScale) > 0.001f)
    {
        ReloadFonts(scale);
        m_prevScale = scale;
    }

    if (m_step == TaskStep::Completed || m_step == TaskStep::None) return;
    if (!m_currentTask) return;

    int scaledTaskTextX = static_cast<int>(kTaskTextX * scale);
    int scaledTaskTextY = static_cast<int>(kTaskTextY * scale);
    int scaledBgBoxWidth = static_cast<int>(kBgBoxWidth * scale);
    int scaledBgBoxHeight = static_cast<int>(kBgBoxHeight * scale);
    int scaledBgBoxPaddingX = static_cast<int>(kBgBoxPaddingX * scale);
    int scaledBgBoxPaddingY = static_cast<int>(kBgBoxPaddingY * scale);
    int scaledTitleFontSize = static_cast<int>(kTitleFontSize * scale);
    int scaledBarMaxWidth = static_cast<int>(kBarMaxWidth * scale);
    int scaledBarHeight = static_cast<int>(kBarHeight * scale);

    // 背景ボックス
    if (m_isTitleAnimFinished || m_titlePosX > -450.0f)
    {
        int bgX = static_cast<int>(m_titlePosX * scale) - scaledBgBoxPaddingX;
        int bgY = scaledTaskTextY - scaledBgBoxPaddingY;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, kBgBoxAlpha);
        DrawBox(bgX, bgY, bgX + scaledBgBoxWidth, bgY + scaledBgBoxHeight, kBgBoxColor, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // タイトル
    DrawStringToHandle(static_cast<int>(m_titlePosX * scale), scaledTaskTextY, m_currentTask->GetTitle().c_str(), 0xFFFFFF, m_titleFont);

    // タスク内容
    if (m_isTitleAnimFinished)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_taskAlpha);

        int taskY = scaledTaskTextY + scaledTitleFontSize + static_cast<int>(10 * scale);
        m_currentTask->DrawTaskUI(scaledTaskTextX, taskY, scale, m_taskAlpha, this);

        // 進捗バー
        int barY = taskY + static_cast<int>(40 * scale);
        int currentBarWidth = static_cast<int>(scaledBarMaxWidth * m_displayedProgress);
        unsigned int barColor = (m_displayedProgress >= 1.0f) ? 0x00ff80 : 0x6496ff;

        DrawBox(scaledTaskTextX, barY, scaledTaskTextX + scaledBarMaxWidth, barY + scaledBarHeight, 0x646464, true);
        DrawBox(scaledTaskTextX, barY, scaledTaskTextX + currentBarWidth, barY + scaledBarHeight, barColor, true);

        DrawStringToHandle(scaledTaskTextX + scaledBarMaxWidth + static_cast<int>(5 * scale), barY, m_currentTask->GetProgressText().c_str(), 0xFFFFFF, m_taskFont);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // 制限アクションフィードバック描画
    if (m_restrictedActionAlpha > 0)
    {
        // 画像サイズ取得
        int designerW = 0, designerH = 0;
        GetGraphSize(m_designerImg, &designerW, &designerH);
        
        float feedbackScale = scale * 0.12f; 
        int targetW = static_cast<int>(designerW * feedbackScale);
        int targetH = static_cast<int>(designerH * feedbackScale); 
        
        int centerX = Game::GetScreenWidth() / 2;
        int centerY = static_cast<int>(Game::GetScreenHeight() * 0.6f);
        
        int drawX = centerX - static_cast<int>(targetW * 0.5f);
        int drawY = centerY - static_cast<int>(targetH * 0.5f);
        
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_restrictedActionAlpha);
        DrawExtendGraph(drawX, drawY, drawX + targetW, drawY + targetH, m_designerImg, true);
        
        int iconImg = -1;
        switch (m_restrictedActionType)
        {
        case AttackType::Shoot:
        case AttackType::Shotgun:
            iconImg = m_mouseLeftImg;
            break;
        case AttackType::Tackle:
            iconImg = m_mouseLeftImg; 
            break;
        case AttackType::ShieldThrow:
            iconImg = m_rKeyImg;
            break;
        case AttackType::Parry:
             iconImg = m_mouseRightGuardImg;
             break;
        default:
            break;
        }
        
        if (iconImg != -1)
        {
            int iconW = static_cast<int>(36 * scale * 1.5f);
            int iconH = static_cast<int>(36 * scale * 1.5f);
            DrawExtendGraph(centerX - static_cast<int>(iconW * 0.5f), drawY + static_cast<int>(targetH * 0.5f) - static_cast<int>(iconH * 0.5f), centerX + static_cast<int>(iconW * 0.5f), drawY + static_cast<int>(targetH * 0.5f) + static_cast<int>(iconH * 0.5f), iconImg, true);
        }
        
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // パリィ説明表示 (一時停止中)
    if (m_isParryTutorialPaused)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
        DrawBox(0, 0, Game::GetScreenWidth(), Game::GetScreenHeight(), 0x000000, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        int centerX = Game::GetScreenWidth() / 2;
        int centerY = Game::GetScreenHeight() / 2;

        std::string text1 = "緑色の攻撃はタイミングよくシールドブロック";
        std::string text2 = "を行うことでパリィできる";

        int text1Width = GetDrawStringWidthToHandle(text1.c_str(), -1, m_titleFont);
        int iconWidth = static_cast<int>(32 * scale);
        int text2Width = GetDrawStringWidthToHandle(text2.c_str(), -1, m_titleFont);

        int totalWidth = text1Width + iconWidth + text2Width;
        int startX = centerX - static_cast<int>(totalWidth * 0.5f);
        int currentY = centerY - static_cast<int>(50 * scale);

        DrawStringToHandle(startX, currentY, text1.c_str(), 0xFFFFFF, m_titleFont);
        DrawExtendGraph(startX + text1Width, currentY, startX + text1Width + iconWidth, currentY + iconWidth, m_mouseRightImg, true);
        DrawStringToHandle(startX + text1Width + iconWidth, currentY, text2.c_str(), 0xFFFFFF, m_titleFont);

        std::string resumeText = "右クリックを押して再開";
        int resumeTextWidth = GetDrawStringWidthToHandle(resumeText.c_str(), -1, m_taskFont);
        DrawStringToHandle(centerX - static_cast<int>(resumeTextWidth * 0.5f), centerY + static_cast<int>(50 * scale), resumeText.c_str(), 0xAAAAAA, m_taskFont);
    }
}
