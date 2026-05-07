#include "SceneManager.h"
#include "DebugUtil.h"
#include "Game.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "SceneBase.h"
#include "SceneGameOver.h"
#include "SceneMain.h"
#include "SceneResult.h"
#include "SceneTitle.h"
#include <DxLib.h>
#include <string>

SceneManager::SceneManager()
    : m_pTitle(nullptr)
    , m_pSceneMain(nullptr)
    , m_pResult(nullptr)
    , m_pGameOver(nullptr)
    , m_pCurrentScene(nullptr)
    , m_pNextScene(nullptr)
    , m_pSceneToChange(nullptr)
    , m_isExternalSceneChange(false)
    , m_loadingDotCount(0)
    , m_loadingAnimTimer(0)
    , m_fadeState(FadeState::Idle)
    , m_fadeAlpha(0)
    , m_fadeSpeed(5)
{
}

SceneManager::~SceneManager()
{
    if (m_pTitle != nullptr)
    {
        delete m_pTitle;
        m_pTitle = nullptr;
    }
    if (m_pSceneMain != nullptr)
    {
        delete m_pSceneMain;
        m_pSceneMain = nullptr;
    }
    if (m_pResult != nullptr)
    {
        delete m_pResult;
        m_pResult = nullptr;
    }
    if (m_pGameOver != nullptr)
    {
        delete m_pGameOver;
        m_pGameOver = nullptr;
    }
    
    // サウンドリソースの解放
    SoundManager::GetInstance()->Release();
}

void SceneManager::Init()
{
    // サウンドリストのロード
    SoundManager::GetInstance()->Load("data/CSV/SoundList.csv");

    // 初期シーンをタイトルシーンに設定
    m_pTitle = new SceneTitle();
    m_pTitle->Init();
    m_pCurrentScene = m_pTitle;
    m_fadeState = FadeState::FadingIn;
    m_fadeAlpha = 255;
}

void SceneManager::Update()
{
    // サウンドの更新（フェード処理など）
    SoundManager::GetInstance()->Update(1.0f / 60.0f);

    // マウスの入力状態を更新
    InputManager::GetInstance()->Update();

    // フェードイン・アウト処理
    if (m_fadeState == FadeState::FadingIn)
    {
        m_fadeAlpha -= m_fadeSpeed;
        if (m_fadeAlpha <= 0)
        {
            m_fadeAlpha = 0;
            m_fadeState = FadeState::Idle;
        }
    }
    else if (m_fadeState == FadeState::FadingOut)
    {
        m_fadeAlpha += m_fadeSpeed;
        if (m_fadeAlpha >= 255)
        {
            m_fadeAlpha = 255;

            // 現在のシーンがSceneMainなら、エフェクトをすべて停止する
            if (SceneMain* mainScene = dynamic_cast<SceneMain*>(m_pCurrentScene))
            {
                mainScene->StopAllEffects();
            }

            // 古いシーンを削除
            if (m_pCurrentScene != nullptr)
            {
                // メンバ変数として保持されているポインタと一致する場合は、
                // 二重解放を防ぐためにメンバ変数をnullptrにする
                if (m_pCurrentScene == m_pTitle) m_pTitle = nullptr;
                if (m_pCurrentScene == m_pSceneMain) m_pSceneMain = nullptr;
                if (m_pCurrentScene == m_pResult) m_pResult = nullptr;
                if (m_pCurrentScene == m_pGameOver) m_pGameOver = nullptr;

                // シーン遷移時にすべてのサウンドを確実に停止させる
                SoundManager::GetInstance()->StopAllSE();

                delete m_pCurrentScene;
                m_pCurrentScene = nullptr;
            }

            m_pCurrentScene = m_pSceneToChange;
            m_pCurrentScene->Init();
            m_pSceneToChange = nullptr;

            // 次のステートへ
            if (m_pCurrentScene->IsLoading())
            {
                m_fadeState = FadeState::Loading;
            }
            else
            {
                m_fadeState = FadeState::FadingIn;
            }

            // シーン遷移直後のフレームで、前のシーンでの入力がトリガーされてしまうのを防ぐため、
            // マウスの入力ログを意図的に更新してトリガー判定を無効化する。
            InputManager::GetInstance()->Update();
        }
    }

    // 現在のシーンを更新 (フェードアウト中は更新しない)
    if (m_pCurrentScene != nullptr && m_fadeState != FadeState::FadingOut)
    {
        m_pNextScene = m_pCurrentScene->Update();
        // ロード完了監視 (シーン更新直後にチェックすることで描画ラグを防ぐ)
        if (m_fadeState == FadeState::Loading && !m_pCurrentScene->IsLoading())
        {
            m_fadeState = FadeState::FadingIn;
        }
    }

    // シーンが変わった場合、フェードアウトを開始
    if (m_pNextScene != nullptr && m_pNextScene != m_pCurrentScene)
    {
        if (m_fadeState == FadeState::Idle)
        {
            m_pSceneToChange = m_pNextScene;
            m_fadeState = FadeState::FadingOut;
        }
    }
}

void SceneManager::Draw()
{
    // 現在のシーンを描画
    if (m_pCurrentScene != nullptr)
    {
        m_pCurrentScene->Draw();
    }

    // フェード処理
    // ロード中はシーン側で描画を行うため、フェード膜は描画しない（フェード膜で隠すとロード画面が見えない）
    if (m_fadeAlpha > 0 && m_fadeState != FadeState::Loading)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawBox(0, 0, Game::GetScreenWidth(), Game::GetScreenHeight(), 0x000000, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // デバッグウィンドウを表示
    DebugUtil::ShowDebugWindow();
}

// 外部からシーン変更をリクエストする関数
void SceneManager::RequestChangeScene(SceneBase* newScene)
{
    if (m_fadeState == FadeState::Idle)
    {
        m_pSceneToChange = newScene;
        m_fadeState = FadeState::FadingOut;
        m_isExternalSceneChange = true; // 外部からの変更要求があったことを示す
    }
}