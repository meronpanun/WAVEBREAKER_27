#include "SceneMain.h"
#include "AmmoItem.h"
#include "AnimationManager.h"
#include "BossUI.h"
#include "PlayerUI.h"
#include "WaveUI.h"
#include "ReticleUI.h"
#include "ScoreUI.h"
#include "HitMarkUI.h"
#include "PlayerEffectUI.h"
#include "UIManager.h"
#include "Camera.h"
#include "DebugUtil.h"
#include "DirectionIndicator.h"
#include "Effect.h"
#include "EffekseerForDXLib.h"
#include "EnemyAcid.h"
#include "EnemyBase.h"
#include "EnemyNormal.h"
#include "EnemyRunner.h"
#include "FirstAidKitItem.h"
#include "ShellCasing.h"
#include "Game.h"
#include "InputManager.h"
#include "Player.h"
#include "SceneManager.h"
#include "SceneGameOver.h"
#include "SceneResult.h"
#include "SceneTitle.h"
#include "ScoreManager.h"
#include "Stage.h"
#include "TaskTutorialManager.h"
#include "TutorialManager.h"
#include "WaveManager.h"
#include "SoundManager.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <string>

// static変数の定義
float SceneMain::s_elapsedTime = 0.0f;
bool SceneMain::s_isSkipTutorial = false;
bool SceneMain::s_hasShownLowHealthTutorial = false;

namespace SceneMainConstants
{
    // UI関連の定数
    constexpr int kButtonWidth = 200;           // ボタンの幅
    constexpr int kButtonHeight = 50;           // ボタンの高さ
    constexpr int kFontSize = 48;               // フォントサイズ
    constexpr float kScreenCenterOffset = 0.5f; // 画面中央のオフセット
    constexpr int kButtonYOffset = 70;          // ボタンのY座標オフセット
    constexpr int kButtonSpacing = 20;          // ボタン間のスペース

    // ゲームクリアシーンへの遷移遅延フレーム数
    constexpr int kClearSceneDelayFrames = 60;

    // 戻るボタンとオプションボタンの座標
    constexpr int kReturnButtonX = 210; // 戻るボタンのX座標
    constexpr int kReturnButtonY = 290; // 戻るボタンのY座標
    constexpr int kOptionButtonX = 210; // オプションボタンのX座標
    constexpr int kOptionButtonY = 120; // オプションボタンのY座標

    // カメラの回転速度
    constexpr float kCameraRotaSpeed = 0.0001f;

    // スカイドーム関連
    constexpr float kSkyDomePosY = 200.0f;  // スカイドームのY座標
    constexpr float kSkyDomeScale = 150.0f; // スカイドームのスケール

    // アイテムドロップ時の初期上昇量
    constexpr float kDropInitialHeight = 140.0f;

    // 環境光設定
    constexpr float kAmbientLightR = 0.5f; // 環境光の赤成分
    constexpr float kAmbientLightG = 0.5f; // 環境光の緑成分
    constexpr float kAmbientLightB = 0.5f; // 環境光の青成分
    constexpr float kAmbientLightA = 1.0f; // 環境光のアルファ成分

    // ヒットマーク関連
    constexpr int kHitMarkLineLength = 8;       // ラインの長さ
    constexpr int kHitMarkCenterSpacing = 4;    // 中央の間隔幅
    constexpr int kHitMarkLineThickness = 2;    // ラインの太さ
    constexpr int kHitMarkDuration = 25;        // 表示時間
    constexpr int kHitMarkDoubleLineOffset = 2; // ダブルラインのオフセット

    // スコアポップアップ関連
    constexpr int kScorePopupX = 80;         // スコアポップアップのX座標
    constexpr int kScorePopupY = 60;         // スコアポップアップのY座標
    constexpr int kPopupOffsetY = 32;        // ポップアップのYオフセット
    constexpr int kPopupDuration = 60;       // 表示時間
    constexpr int kTotalScoreDuration = 120; // 合計スコアの表示時間

    // レティクル表示位置補正値
    constexpr int kReticleOffset = 64;

    // RoadFloorオブジェトの範囲
    constexpr VECTOR kRoadFloorMin = { -500.0f, 0.0f, -500.0f }; // 床の最小座標
    constexpr VECTOR kRoadFloorMax = { 500.0f, 0.0f, 500.0f };   // 床の最大座標

	// サウンド関連の定数(0〜255の音量範囲)
    constexpr int kGameSceneBgmVolume = 100; // ゲームシーンBGMの音量 
    constexpr int kHeadShotSEVolume   = 255; // ヘッドショット時のSE音量
}

using namespace SceneMainConstants;

SceneMain* g_sceneMainInstance = nullptr;

SceneMain* SceneMain::Instance() { return g_sceneMainInstance; }

SceneMain::SceneMain(bool isReturningFromOtherScene)
    : m_isPaused(false)
    , m_isEscapePressed(false)
    , m_isReturningFromOtherScene(isReturningFromOtherScene)
    , m_cameraSensitivity(Game::g_cameraSensitivity)
    , m_hitDistance(0.0f)
    , m_pCamera(std::make_unique<Camera>())
    , m_hasDroppedWave1FirstAid(false)
    , m_hasDroppedWave1Ammo(false)
    , m_wave1DropCount(0)
    , m_isLoading(true)
    , m_headShotSECooldownTimer(0)
    , m_isPlayerInit(false)
    , m_isTaskTutorialInit(false)
    , m_pEffect(std::make_unique<Effect>())
    , m_pAnimManager(std::make_unique<AnimationManager>())
    , m_gameOverDelayTimer(-1)
    , m_isTutorialStage(false)
    , m_loadingFrameCount(0)
    , m_loadingDotCount(0)
    , m_loadingAnimTimer(0)
    , m_loadingModelPos(VGet(0, 0, 0))
    , m_loadingModelAnimTime(0.0f)
    , m_isShowDebugHUD(false)
    , m_lastDeltaTime(0.0f)
    , m_prevTimeCount(GetNowHiPerformanceCount())
    , m_cachedFPS(0.0f)
    , m_cachedDeltaTime(0.0f)
    , m_cachedTotalEnemies(0)
    , m_cachedUpdatedEnemies(0)
    , m_cachedDrawnEnemies(0)
    , m_debugDisplayTimer(0)
{
    g_sceneMainInstance = this;
    
    // ManagedFontによりコンストラクタでロードされるため、ここでのロードは不要
}

SceneMain::~SceneMain()
{
    // 全てのSEを停止
    SoundManager::GetInstance()->StopAllSE();

    // アイテムモデルの解放
    FirstAidKitItem::DeleteModel();
    AmmoItem::DeleteModel();
    ShellCasing::DeleteResources();

    // 自動解放されるため処理不要

    // インジケーター画像の解放
    DirectionIndicator::DeleteResources();
}

void SceneMain::Init()
{
    // 経過時間リセット
    s_elapsedTime = 0.0f;
    m_isPlayerInit = false; // プレイヤー初期化フラグをリセット
    m_prevTimeCount = GetNowHiPerformanceCount(); // 時間計測を現在時刻から再開

    // ローディング用モデルの読み込み（非同期ロードの前に同期で読み込む）
    m_loadingModel.Reset(MV1LoadModel("data/model/NormalZombie.mv1"));
    // 初期設定
    // 歩行アニメーションを名前で検索
    int walkAnimIndex = MV1GetAnimIndex(m_loadingModel, "WALK");
    if (walkAnimIndex == -1) walkAnimIndex = 0; // 見つからなければ0番

    // アニメーションをアタッチ (slot 0)
    MV1AttachAnim(m_loadingModel, walkAnimIndex, -1, FALSE);

    m_loadingModelPos = VGet(-200.0f, -750.0f, 600.0f); // 画面左外側、カメラスペースでの位置 (さらに下げる)
    m_loadingModelAnimTime = 0.0f;
    // スケール調整
    MV1SetScale(m_loadingModel, VGet(1.0f, 1.0f, 1.0f));
    // 回転 (右向きに修正: 90度で左だったため-90度に変更)
    MV1SetRotationXYZ(m_loadingModel, VGet(0.0f, -90.0f * DX_PI_F / 180.0f, 0.0f));

    // 非同期読み込みを有効化
    SetUseASyncLoadFlag(true);

    // アイテムモデルの読み込み
    FirstAidKitItem::LoadModel();
    AmmoItem::LoadModel();
    ShellCasing::LoadResources();

    // インジケーター画像の読み込み
    DirectionIndicator::LoadResources();

    // 重いリソースの非同期読み込みを開始
    m_skyDome.Reset(MV1LoadModel("data/model/Dome.mv1"));
    // UI管理クラスの初期化
    m_pUIManager = std::make_unique<UIManager>();

    m_pPlayer = std::make_unique<Player>();
    m_pPlayer->SetEffect(m_pEffect.get());
    m_pPlayer->SetAnimationManager(m_pAnimManager.get());
    Game::m_pPlayer = m_pPlayer.get();

    m_pEnemyNormal = std::make_shared<EnemyNormal>();
    m_pEnemyNormal->Init();

    m_pEnemyRunner = std::make_shared<EnemyRunner>();
    m_pEnemyRunner->Init();

    m_pEnemyAcid = std::make_shared<EnemyAcid>();
    m_pEnemyAcid->Init();

    m_pStage = std::make_shared<Stage>();

    // チュートリアルスキップフラグに応じてステージをロード
    if (s_isSkipTutorial || m_isReturningFromOtherScene)
    {
        m_pStage->LoadStage(false); // メインステージ
        m_isTutorialStage = false;
    }
    else
    {
        m_pStage->LoadStage(true); // チュートリアルステージ
        m_isTutorialStage = true;
    }

    m_pWaveManager = std::make_shared<WaveManager>();
    m_pWaveManager->Init();
    Game::m_pWaveManager = m_pWaveManager.get();

    // 方向インジケーターの初期化
    m_pDirectionIndicator = std::make_unique<DirectionIndicator>();
    m_pDirectionIndicator->Init(m_pPlayer.get());

    // UIコンポーネントの登録
    auto playerUI = std::make_shared<PlayerUI>(m_pPlayer.get());
    auto waveUI = std::make_shared<WaveUI>(m_pWaveManager.get());
    auto bossUI = std::make_shared<BossUI>(m_pWaveManager.get());
    auto reticleUI = std::make_shared<ReticleUI>(m_pPlayer.get());
    auto scoreUI = std::make_shared<ScoreUI>();
    auto hitMarkUI = std::make_shared<HitMarkUI>();
    auto playerEffectUI = std::make_shared<PlayerEffectUI>(m_pPlayer.get());

    m_pUIManager->AddUI(playerUI);
    m_pUIManager->AddUI(waveUI);
    m_pUIManager->AddUI(bossUI);
    m_pUIManager->AddUI(reticleUI);
    m_pUIManager->AddUI(scoreUI);
    m_pUIManager->AddUI(hitMarkUI);
    m_pUIManager->AddUI(playerEffectUI);
    
    // チュートリアルステージの場合はWaveUIを非表示にする
    if (m_isTutorialStage)
    {
        waveUI->SetVisible(false);
    }

    m_pUIManager->Init();

    // RoadFloorオブジェクトの範囲を設定（マップ全体の範囲）
    m_pWaveManager->SetRoadFloorBounds(m_pStage->GetMinBounds(), m_pStage->GetMaxBounds());
    m_pWaveManager->RegisterStageToGrid(m_pStage->GetCollisionData());
    m_pWaveManager->GetCollisionGrid().CalculateHeights(m_pStage->GetCollisionData());

    // カメラの初期化
    if (m_pPlayer->GetCamera())
    {
        m_pPlayer->GetCamera()->SetSensitivity(m_cameraSensitivity);
    }

    // マウスカーソルの表示/非表示を設定
    SetMouseDispFlag(m_isPaused);

    // スカイドームのY座標を設定
    MV1SetPosition(m_skyDome, VGet(0, kSkyDomePosY, 0));

    // スカイドームのスケールを設定
    MV1SetScale(m_skyDome, VGet(kSkyDomeScale, kSkyDomeScale, kSkyDomeScale));


    m_items.clear();

    // wave1開始時にフラグとカウントをリセット
    m_hasDroppedWave1FirstAid = false;
    m_hasDroppedWave1Ammo = false;
    m_wave1DropCount = 0;

    m_clearSceneDelayTimer = -1; // 遅延タイマーをリセット

    // WaveManagerの敵の死亡時にアイテムをドロップするコールバックを設定
    m_pWaveManager->SetOnEnemyDeathCallback([this](const VECTOR &pos)
    {
        static VECTOR lastDropPos = { -99999, -99999, -99999 };
        // 直前と同じ座標なら何もしない
        if (pos.x == lastDropPos.x && pos.y == lastDropPos.y && pos.z == lastDropPos.z) return;
        lastDropPos = pos;
        if (m_pWaveManager->GetCurrentWave() == 1)
        {
            if (m_wave1DropCount >= 2) return; // 0.5f体分だけドロップ

            if (!m_hasDroppedWave1FirstAid && !m_hasDroppedWave1Ammo) 
            {
                int randValue = GetRand(99);
                if (randValue < 50) 
                {
                    auto firstAid = std::make_shared<FirstAidKitItem>();
                    firstAid->Init();
                    VECTOR dropPos = pos;
                    dropPos.y += kDropInitialHeight;
                    firstAid->SetPos(dropPos);
                    m_items.push_back(firstAid);
                    m_hasDroppedWave1FirstAid = true;
                }
                else 
                {
                    auto ammo = std::make_shared<AmmoItem>();
                    ammo->Init();
                    VECTOR dropPos = pos;
                    dropPos.y += kDropInitialHeight;
                    ammo->SetPos(dropPos);
                    m_items.push_back(ammo);
                    m_hasDroppedWave1Ammo = true;
                }
                m_wave1DropCount++;
            } 
            else if (!m_hasDroppedWave1FirstAid) 
            {
                auto firstAid = std::make_shared<FirstAidKitItem>();
                firstAid->Init();
                VECTOR dropPos = pos;
                dropPos.y += kDropInitialHeight;
                firstAid->SetPos(dropPos);
                m_items.push_back(firstAid);
                m_hasDroppedWave1FirstAid = true;
                m_wave1DropCount++;
            }
            else if (!m_hasDroppedWave1Ammo) 
            {
                auto ammo = std::make_shared<AmmoItem>();
                ammo->Init();
                VECTOR dropPos = pos;
                dropPos.y += kDropInitialHeight;
                ammo->SetPos(dropPos);
                m_items.push_back(ammo);
                m_hasDroppedWave1Ammo = true;
                m_wave1DropCount++;
            }
            // 両方ドロップ済み or 2体分超えたら何も落とさない
        } 
        else 
        {
            // wave2以降はどちらか一方のみドロップ
            int randValue = GetRand(99);
            std::shared_ptr<ItemBase> dropItem;
            if (randValue < 50) 
            {
                dropItem = std::make_shared<FirstAidKitItem>();
            }
            else 
            {
                dropItem = std::make_shared<AmmoItem>();
            }
            dropItem->Init();
            VECTOR dropPos = pos;
            dropPos.y += kDropInitialHeight;
            dropItem->SetPos(dropPos);
            m_items.push_back(dropItem);
        }
    });

    // チュートリアルマネージャ生成・初期化
    m_pTutorialManager = std::make_unique<TutorialManager>();
    if (!m_isReturningFromOtherScene && !s_isSkipTutorial) 
    {
        m_pTutorialManager->Init();
    }

    // ヒットマーク用コールバックをWaveManagerに  // 敵ヒット時のコールバック設定
    m_pWaveManager->SetOnEnemyHitCallback(
        [this](EnemyBase::HitPart part, float distance)
        {
            OnPlayerBulletHitEnemy(part, distance);
        });

    // 環境光の設定
    SetLightAmbColor(GetColorF(kAmbientLightR, kAmbientLightG, kAmbientLightB, kAmbientLightA));


    // チュートリアルマネージャーをリセットまたはスキップ
    if (m_isReturningFromOtherScene) 
    {
        TaskTutorialManager::GetInstance()->Skip(m_pWaveManager.get());
    } 
    else 
    {
        TaskTutorialManager::GetInstance()->Reset();
    }
}

// スコアポップアップを追加する
void SceneMain::AddScorePopup(int score, bool isHeadShot, int combo) 
{
    if (m_pUIManager)
    {
        auto scoreUI = m_pUIManager->GetUI<ScoreUI>();
        if (scoreUI)
        {
            scoreUI->AddScorePopup(score, isHeadShot);
        }
    }
}

void SceneMain::SwitchToMainStage()
{
  // チュートリアルのアイテムを消去
  m_items.clear();

  // WaveManagerをリセット (敵の消去とWave情報の初期化)
  m_pWaveManager->Reset();

  // エフェクトを全て停止
  if (m_pEffect) {
    m_pEffect->StopAllEffects();
  }

  // メインステージをロード
  m_pStage->LoadStage(false);
  m_isTutorialStage = false;

  // ステージ範囲を更新
  m_pWaveManager->SetRoadFloorBounds(m_pStage->GetMinBounds(), m_pStage->GetMaxBounds());
  m_pWaveManager->RegisterStageToGrid(m_pStage->GetCollisionData());
  m_pWaveManager->GetCollisionGrid().CalculateHeights(m_pStage->GetCollisionData());

  // プレイヤーの再初期化（位置などをCSVから再取得）
  m_pPlayer->Init(false);

  // WaveUIを表示する
  auto waveUI = m_pUIManager->GetUI<WaveUI>();
  if (waveUI)
  {
      waveUI->SetVisible(true);
  }
}

SceneBase* SceneMain::Update()
{
    // ローディング中は他の処理を行わない
    if (m_isLoading)
    {
        // ローディングアニメーション更新
        m_loadingAnimTimer++;
        if (m_loadingAnimTimer > 30)
        {
            m_loadingAnimTimer = 0;
            m_loadingDotCount++;
            if (m_loadingDotCount > 3)
            {
                m_loadingDotCount = 0;
            }
        }

        // ローディングモデルの更新 (歩行アニメーション)
        if (m_loadingModel.IsValid())
        {
            // アニメーション進行
            m_loadingModelAnimTime += 1.0f; // スピード調整
            float totalTime = MV1GetAttachAnimTotalTime(m_loadingModel, 0);
            if (m_loadingModelAnimTime >= totalTime)
            {
                m_loadingModelAnimTime = fmodf(m_loadingModelAnimTime, totalTime);
            }
            MV1SetAttachAnimTime(m_loadingModel, 0, m_loadingModelAnimTime);

            // 移動 (左から右へ)
            m_loadingModelPos.x += 3.0f;
            if (m_loadingModelPos.x > 350.0f) // 画面右端を超えたらループ
            {
                m_loadingModelPos.x = -350.0f;
            }

            // 位置設定 (固定カメラを前提とした簡易配置)
        }

        // 非同期読み込みが完了しているかチェック
        if (GetASyncLoadNum() == 0)
        {
            // 最低限のローディング時間を確保
            m_loadingFrameCount++;
            if (m_loadingFrameCount >= 80)
            {
                m_isLoading = false;
                m_loadingFrameCount = 0;

                // 非同期ロード完了後に1回だけ呼ぶ
                if (!m_isPlayerInit && m_pPlayer)
                {
                    m_pPlayer->Init(m_isTutorialStage);
                    m_isPlayerInit = true;

                    // 非同期読み込みを無効化
                    SetUseASyncLoadFlag(false);
                }
            }
            else
            {
                // グレース期間中もローディング中とみなし、処理を返して真っ暗な画面での進行を防ぐ
                // Draw()でのローディング表示を有効にするためにここを通す
                return this;
            }
        }
        else
        {
            return this;
        }
    }

    // BGM再生
    if (!m_isLoading) // ロード完了後に再生開始
    {
        SoundManager::GetInstance()->PlayBGM("BGM", "Main");
    }

    // タイムスケールの更新
    Game::UpdateTimeScale();

    // ヘッドショットSEのクールタイム更新
    if (m_headShotSECooldownTimer > 0) m_headShotSECooldownTimer--;

    // 経過時間を加算
    long long now = GetNowHiPerformanceCount();
    m_lastDeltaTime = (now - m_prevTimeCount) / 1000000.0f;
    m_prevTimeCount = now;

    float dt = (1.0f / 60.0f) * Game::GetTimeScale();
    s_elapsedTime += dt;

    // UIの更新 (ポーズ中やチュートリアル中もタイマーを進める、またはスケールを更新するためにここで呼ぶ)
    if (m_pUIManager)
    {
        m_pUIManager->Update(m_lastDeltaTime);
    }

    // デバックウィンドウが表示されている場合は、更新をスキップ
    if (DebugUtil::IsDebugWindowVisible())
    {
        return this;
    }

    // スカイドームの回転
    MV1SetRotationXYZ(m_skyDome, VGet(0, MV1GetRotationXYZ(m_skyDome).y + kCameraRotaSpeed, 0));

    // エスケープキーが押されたかチェック
    if (CheckHitKey(KEY_INPUT_ESCAPE))
    {
        if (!m_isEscapePressed)
        {
            m_isPaused = !m_isPaused;
            SetMouseDispFlag(m_isPaused);
            m_isEscapePressed = true;

            if (m_isPaused)
            {
                m_pauseStartTime = std::chrono::steady_clock::now();
            }
            else
            {
                auto now = std::chrono::steady_clock::now();
                auto pauseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_pauseStartTime).count();

                // ポーズ解除時にマウスを中央にリセットし、視点のかくつきを防ぐ
                SetMousePoint(static_cast<int>(Game::GetScreenWidth() * 0.5f), static_cast<int>(Game::GetScreenHeight() * 0.5f));
            }
        }
    }
    else
    {
        m_isEscapePressed = false;
    }

    if (m_isPaused)
    {
        if (InputManager::GetInstance()->IsTriggerMouseLeft())
        {
            Vec2 mousePos = InputManager::GetInstance()->GetMousePos();

            if (mousePos.x >= kReturnButtonX &&
                mousePos.x <= kReturnButtonX + kButtonWidth &&
                mousePos.y >= kReturnButtonY &&
                mousePos.y <= kReturnButtonY + kButtonHeight)
            {
                // BGMを停止
                SoundManager::GetInstance()->StopBGM();
                return new SceneTitle(true);
            }

        }
        return this;
    }

    // チュートリアルマネージャの更新
    if (m_pTutorialManager)
    {
        m_pTutorialManager->Update();

        // 低体力チュートリアルの表示
        if (m_pPlayer && m_pPlayer->IsLowHealth() && !s_hasShownLowHealthTutorial)
        {
            m_pTutorialManager->AddMessage("回復アイテム",
                                           "敵を倒すと回復アイテムをドロップする。\n"
                                           "生き残りたければ積極的に行動せよ");
            s_hasShownLowHealthTutorial = true;
        }
    }

    // 基本操作チュートリアル中の処理
    if (m_pTutorialManager && m_pTutorialManager->IsActive())
    {
        m_pPlayer->Update({}, m_pStage->GetCollisionData()); // プレイヤーはチュートリアル中も移動可能
        return this;
    }
    // 基本操作チュートリアルが完了したら、タスクチュートリアルを初期化
    else if (m_pTutorialManager && m_pTutorialManager->IsCompleted() && !m_isTaskTutorialInit)
    {
        TaskTutorialManager::GetInstance()->Init(m_pWaveManager.get(), m_pPlayer.get());
        m_isTaskTutorialInit = true;
    }

    // タスクチュートリアルが完了していない間
    if (!TaskTutorialManager::GetInstance()->IsCompleted())
    {
        TaskTutorialManager::GetInstance()->Update();

        // タスクチュートリアル中もWaveManagerの更新（スポーン処理のみ）を行う
        m_pWaveManager->Update();

        // タスクチュートリアル中はWaveManagerの通常の更新は行わない
        // ただし、敵の更新とプレイヤーの更新は必要

        // WaveManagerからアクティブな敵のリストを取得してプレイヤーを更新
        std::vector<std::shared_ptr<EnemyBase>>& enemyList = m_pWaveManager->GetEnemyList();
        std::vector<EnemyBase*> enemyPtrList;
        for (std::shared_ptr<EnemyBase>& enemy : enemyList)
        {
            enemyPtrList.push_back(enemy.get());
        }
        // プレイヤーは敵を撃ったりタックルしたりするために敵で更新する必要がある
        m_pPlayer->Update(enemyPtrList, m_pStage->GetCollisionData());
        
        // 敵も更新する必要がある
        m_pWaveManager->UpdateEnemies(m_pPlayer->GetBullets(), m_pPlayer->GetTackleInfo(), *m_pPlayer,
            m_pStage->GetCollisionData(), m_pEffect.get());

        // アイテム、スコアポップアップなどの更新はタスクチュートリアル中でも実行
        for (std::shared_ptr<ItemBase>& item : m_items)
        {
            item->Update(m_pPlayer.get(), m_pStage->GetCollisionData());
        }
        m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
            [](const std::shared_ptr<ItemBase>& item) { return item->IsUsed() || item->IsExpired(); }), m_items.end());

        ScoreManager::Instance().Update();

        // ゲームオーバーチェックもここで実行
        if (m_pPlayer->IsDead())
        {
            if (m_gameOverDelayTimer == -1)
            {
                m_gameOverDelayTimer = 180; // 3秒の遅延
            }
            else if (m_gameOverDelayTimer > 0)
            {
                m_gameOverDelayTimer--;
            }
            else
            {
                int wave = m_pWaveManager->GetCurrentWave();
                int killCount = ScoreManager::Instance().GetBodyKillCount() + ScoreManager::Instance().GetHeadKillCount();
                int score = ScoreManager::Instance().GetTotalScore();
                SoundManager::GetInstance()->StopBGM();
                return new SceneGameOver(wave, killCount, score);
            }
        }

        m_pDirectionIndicator->Update(m_pWaveManager->GetEnemyList()); // 方向インジケータも更新

        return this; // タスクチュートリアル中に留まる
    }

    // ここから通常進行 (両方のチュートリアルが完了した場合のみ)
    // チュートリアルステージにいる場合はメインステージに切り替え
    if (m_isTutorialStage)
    {
        SwitchToMainStage();
    }

    m_pWaveManager->Update(); // メインのウェーブマネージャを更新

    std::vector<std::shared_ptr<EnemyBase>>& enemyList = m_pWaveManager->GetEnemyList();
    std::vector<EnemyBase*> enemyPtrList;
    for (std::shared_ptr<EnemyBase>& enemy : enemyList)
    {
        enemyPtrList.push_back(enemy.get());
    }
    m_pPlayer->Update(enemyPtrList, m_pStage->GetCollisionData());

    if (m_pPlayer->IsDead())
    {
        if (m_gameOverDelayTimer == -1)
        {
            m_gameOverDelayTimer = 180;
        }
        else if (m_gameOverDelayTimer > 0)
        {
            m_gameOverDelayTimer--;
        }
        else
        {
            int wave = m_pWaveManager->GetCurrentWave();
            int killCount = ScoreManager::Instance().GetBodyKillCount() + ScoreManager::Instance().GetHeadKillCount();
            int score = ScoreManager::Instance().GetTotalScore();
            SoundManager::GetInstance()->StopBGM();
            return new SceneGameOver(wave, killCount, score);
        }
    }

    // ウェーブ5終了後の遅延処理
    if (m_pWaveManager->GetCurrentWave() > 5)
    {
        if (m_clearSceneDelayTimer == -1) // 遅延がまだ開始されていない場合
        {
            m_clearSceneDelayTimer = kClearSceneDelayFrames; // 遅延タイマーを開始
            SoundManager::GetInstance()->StopBGM();                       // BGMを停止
        }
        else if (m_clearSceneDelayTimer > 0) // 遅延中の場合
        {
            m_clearSceneDelayTimer--; // タイマーを減らす
        }
        else // 遅延が終了した場合
        {
            return new SceneResult(); // シーン遷移
        }
    }

    // 遅延中は他の処理をスキップ
    if (m_clearSceneDelayTimer != -1) return this;
    // AI更新カウンタのリセット
    EnemyBase::ResetAIUpdateCount();
    EnemyBase::ResetTotalCount();

    m_pWaveManager->UpdateEnemies(m_pPlayer->GetBullets(), m_pPlayer->GetTackleInfo(),
        *m_pPlayer, m_pStage->GetCollisionData(), m_pEffect.get());

    for (std::shared_ptr<ItemBase>& item : m_items)
    {
        item->Update(m_pPlayer.get(), m_pStage->GetCollisionData());
    }
    m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
        [](const std::shared_ptr<ItemBase>& item) { return item->IsUsed() || item->IsExpired(); }), m_items.end());

    m_pDirectionIndicator->Update(m_pWaveManager->GetEnemyList()); // 方向インジケータも更新
    ScoreManager::Instance().Update();

    return this;
}

bool SceneMain::IsLoading() const { return m_isLoading; }

void SceneMain::Draw()
{
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    EnemyBase::ResetDrawCount();

    m_pStage->Draw();

    MV1DrawModel(m_skyDome);

    for (std::shared_ptr<ItemBase>& item : m_items)
    {
        item->Draw();
    }

    // ローディング中の表示
    if (m_isLoading)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
        DrawBox(0, 0, screenW, screenH, 0x000000, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        SetFontSize(static_cast<int>(48 * Game::GetUIScale()));
        std::string loadingText = "Now Loading";
        for (int i = 0; i < m_loadingDotCount; ++i)
        {
            loadingText += ".";
        }

        int textWidth = GetDrawStringWidth(loadingText.c_str(), -1);
        int textX = (screenW - textWidth) * 0.5f;
        float scale = Game::GetUIScale();
        int textY = (screenH - static_cast<int>(48 * scale)) * 0.5f - static_cast<int>(50 * scale); // テキストを少し上にずらす
        DrawString(textX, textY, loadingText.c_str(), 0xffffff);
        SetFontSize(16);

        // ローディングモデルの描画
        // ローディングモデルの描画
        if (m_loadingModel.IsValid())
        {
            // カメラ設定をローディング専用にする
            VECTOR camPos = VGet(0.0f, -550.0f, 0.0f);

            VECTOR camTarget = VGet(0.0f, -550.0f, 1000.0f);
            SetupCamera_Perspective(60.0f * DX_PI_F / 180.0f);
            SetCameraPositionAndTarget_UpVecY(camPos, camTarget);

            // モデル位置設定
            MV1SetPosition(m_loadingModel, m_loadingModelPos);
            MV1DrawModel(m_loadingModel);

            // カメラ設定を戻す (次のフレームの描画に影響しないように推奨されるが、
            // 実際にはメインループで毎フレーム設定されるため、ここでは簡易的で良い)
        }

        return; // ローディング中はこれ以降描画しない
    }

    // 敵の描画
    m_pWaveManager->DrawEnemies(m_pStage->GetCollisionData(), m_isTutorialStage);

    // エフェクトの描画
    m_pEffect->Draw();

    m_pPlayer->Draw3D();

    // ここからUI描画
    m_pPlayer->DrawShield();

    // チュートリアルUI（最前面に表示）
    if (m_pTutorialManager)
    {
        m_pTutorialManager->Draw(screenW, screenH);
    }

    if (m_pTutorialManager && m_pTutorialManager->IsActive())
    {
        // ...
    }
    else if (!TaskTutorialManager::GetInstance()->IsCompleted())
    {
        TaskTutorialManager::GetInstance()->Draw();
    }

    // UIの一括描画
    m_pUIManager->Draw();

    // ポーズメニューの描画
    if (m_isPaused)
    {
        DrawPauseMenu();
    }

    // デバッグHUD描画
    if (m_isShowDebugHUD)
    {
        DrawDebugHUD();
    }

    // 空間分割デバッグUI描画
    if (m_pWaveManager)
    {
        m_pWaveManager->DrawDebugUI();
    }
}

void SceneMain::SetShowDebugHUD(bool show)
{
    m_isShowDebugHUD = show;
    // EnemyBase側のフラグも連動させる
    EnemyBase::SetShowDamage(show);
}

void SceneMain::DrawDebugHUD()
{
    int screenW = Game::GetScreenWidth();
    int screenH = Game::GetScreenHeight();

    // 画面全体を半透明の黒で覆う
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
    DrawBox(0, 0, screenW, screenH, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 情報収集
    VECTOR playerPos = m_pPlayer ? m_pPlayer->GetPos() : VGet(0, 0, 0);

    // 表示更新頻度を落とす
    m_debugDisplayTimer--;
    if (m_debugDisplayTimer <= 0)
    {
        m_cachedFPS = GetFPS();
        m_cachedDeltaTime = m_lastDeltaTime;
        m_cachedTotalEnemies = EnemyBase::GetTotalCount();
        m_cachedUpdatedEnemies = EnemyBase::GetAIUpdateCount();
        m_cachedDrawnEnemies = EnemyBase::GetDrawCount();
        m_debugDisplayTimer = 15;
    }
    float fps = m_cachedFPS;
    int aliveEnemyCount = m_pWaveManager ? m_pWaveManager->GetAliveEnemyCount() : 0;
    int drawnEnemyCount = m_cachedDrawnEnemies;

    // Last Damage
    float lastDamage = EnemyBase::GetDebugLastDamage();
    std::string hitInfo = EnemyBase::GetDebugHitInfo();
    int damageTimer = EnemyBase::GetDebugDamageTimer();
    std::string damageStr = (damageTimer > 0) ? std::to_string((int)lastDamage) + (hitInfo.empty() ? "" : " " + hitInfo) : "-";

    // テキスト描画 (左上)
    int x = 20;
    int y = 20;
    int lineHeight = 20;
    unsigned int color = 0xFFFFFF;

    DrawFormatString(x, y, color, "FPS: %.1f", fps);
    y += lineHeight;
    DrawFormatString(x, y, color, "Delta Time: %.4f", m_cachedDeltaTime);
    y += lineHeight;
    DrawFormatString(x, y, color, "Player Pos: (%.1f, %.1f, %.1f)", playerPos.x,
                     playerPos.y, playerPos.z);
    y += lineHeight;
    if (m_pPlayer)
    {
        DrawFormatString(x, y, color, "Speed: %.2f", m_pPlayer->GetCurrentSpeed());
        y += lineHeight;
    }

    DrawFormatString(x, y, color, "Active Enemy Count: %d", aliveEnemyCount);
    y += lineHeight;
    DrawFormatString(x, y, color, "Drawn Enemy Count: %d", drawnEnemyCount);
    y += lineHeight;

    // AI間引き(Throttling)の評価
    int totalCount = m_cachedTotalEnemies;
    int aiUpdatedCount = m_cachedUpdatedEnemies;
    float throttlingRate = totalCount > 0 ? (1.0f - (float)aiUpdatedCount / totalCount) * 100.0f : 0.0f;
    DrawFormatString(x, y, color, "AI Update: %d/%d (Throttled: %.1f%%)", aiUpdatedCount, totalCount, throttlingRate);
    y += lineHeight;

    // 描画カリングの評価
    float cullingRate = totalCount > 0 ? (1.0f - (float)drawnEnemyCount / totalCount) * 100.0f : 0.0f;
    DrawFormatString(x, y, color, "Draw Culling: %.1f%%", cullingRate);
    y += lineHeight;

    DrawFormatString(x, y, color, "Total Defeated: %d", ScoreManager::Instance().GetTotalDefeatedCount());
    y += lineHeight;
    DrawFormatString(x, y, color, "Last Damage: %s", damageStr.c_str());
    y += lineHeight;
}

void SceneMain::DrawPauseMenu()
{
    Vec2 mousePos = InputManager::GetInstance()->GetMousePos();

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
    DrawBox(50, 50, Game::GetScreenWidth() - 50, Game::GetScreenHeight() - 50, 0x000000, true);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void SceneMain::SetPaused(bool paused)
{
    m_isPaused = paused;
    SetMouseDispFlag(m_isPaused);
}

// カメラの感度を設定
void SceneMain::SetCameraSensitivity(float sensitivity)
{
    m_cameraSensitivity = sensitivity;
    if (m_pCamera)
    {
        m_pCamera->SetSensitivity(sensitivity);
    }
}

/// <summary>
/// プレイヤーの弾が敵にヒットした際に呼ばれる
/// </summary>
void SceneMain::OnPlayerBulletHitEnemy(EnemyBase::HitPart part, float distance)
{
    // ヒットの部位によってSE再生
    if (part == EnemyBase::HitPart::Head)
    {
        if (m_headShotSECooldownTimer <= 0)
        {
            SoundManager::GetInstance()->Play("UI", "HeadShot");
            m_headShotSECooldownTimer = 10;
        }
    }

    auto hitMarkUI = m_pUIManager->GetUI<HitMarkUI>();
    if (hitMarkUI)
    {
        hitMarkUI->Trigger(part, distance);
    }
}

void SceneMain::StopAllEffects()
{
    if (m_pEffect)
    {
        m_pEffect->StopAllEffects();
    }
}
