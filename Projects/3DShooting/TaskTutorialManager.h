#pragma once
#include "AttackType.h"
#include "ManagedFont.h"
#include "ManagedGraph.h"
#include <memory>

class WaveManager;
class Player;
class ITutorialTask;

/// <summary>
/// タスク型チュートリアルマネージャークラス
/// </summary>
class TaskTutorialManager
{
public:
    TaskTutorialManager();
    ~TaskTutorialManager();

    void Init(WaveManager* pWaveManager, Player* pPlayer);
    void Update();
    void Draw();

    // シングルトンインスタンスを取得
    static TaskTutorialManager* GetInstance();

    // 敵が倒されたことを通知する
    void NotifyEnemyKilled(AttackType attackType);

    // 盾投げで敵が倒されたことを通知する
    void NotifyShieldThrowKill();

    // パリィ成功を通知する
    void NotifyParrySuccess();

    // パリィ可能な攻撃が来たことを通知する（チュートリアル停止用）
    void NotifyParryableAttack();

    // チュートリアルが完了したか
    bool IsCompleted() const;

    // 状態をリセットする
    void Reset();

    // チュートリアルをスキップする
    void Skip(WaveManager* pWaveManager);

    // パリィタスクまでチュートリアルをスキップする
    void SkipToParry();

    // 制限されたアクションが行われたことを通知する
    void NotifyRestrictedAction(AttackType attemptedType);

    // スケール変更時のフォントリロード
    void ReloadFonts(float scale);

    // タスクから利用するゲッター
    int GetDiamondImg() const { return m_diamondImg; }
    int GetMouseLeftImg() const { return m_mouseLeftImg; }
    int GetMouseRightImg() const { return m_mouseRightImg; }
    int GetAlpha1Img() const { return m_alpha1Img; }
    int GetAlpha2Img() const { return m_alpha2Img; }
    int GetMouseWheelImg() const { return m_mouseWheelImg; }
    int GetRKeyImg() const { return m_rKeyImg; }
    int GetLockOnUIImg() const { return m_lockOnUIImg; }
    int GetTaskFont() const { return m_taskFont; }

private:
    // チュートリアルの進行ステップ
    enum class TaskStep
    {
        None,
        Shoot,                    // 射撃タスク
        ShootCompleteDelay,       // 射撃タスク完了後の待機
        Tackle,                   // タックルタスク
        TackleCompleteDelay,      // タックルタスク完了後の待機
        ShieldThrow,              // 盾投げタスク
        ShieldThrowCompleteDelay, // 盾投げタスク完了後の待機
        Parry,                    // パリィタスク
        ParryCompleteDelay,       // パリィタスク完了後の待機
        Completed                 // 全て完了
    };
    TaskTutorialManager(const TaskTutorialManager&) = delete;
    TaskTutorialManager& operator=(const TaskTutorialManager&) = delete;

    static TaskTutorialManager* m_instance;

    WaveManager* m_pWaveManager; // WaveManagerへのポインタ
    Player* m_pPlayer;           // Playerへのポインタ

    TaskStep m_step;
    std::unique_ptr<ITutorialTask> m_currentTask; // 現在実行中のタスク
    
    // 進捗表示用のアニメーション変数
    float m_displayedProgress;
    float m_progressAnimSpeed;            // 進捗バーのアニメーション速度

    ManagedFont m_titleFont; // タイトル用のフォントハンドル
    ManagedFont m_taskFont;  // タスク内容用のフォントハンドル
    
    ManagedGraph m_diamondImg;
    ManagedGraph m_mouseLeftImg;
    ManagedGraph m_mouseRightImg;

    // 武器切り替えヒント用画像
    ManagedGraph m_alpha1Img;     // キーボード1キーの画像
    ManagedGraph m_alpha2Img;     // キーボード2キーの画像
    ManagedGraph m_mouseWheelImg; // マウスホイールの画像

    // 盾投げ・パリィタスク用画像
    ManagedGraph m_rKeyImg;            // Rキーの画像
    ManagedGraph m_lockOnUIImg;        // ロックオンUIの画像
    ManagedGraph m_mouseRightGuardImg; // マウス右クリック(ガード用)の画像
    ManagedGraph m_designerImg;        // Designer.png画像

    // タイトルアニメーション用
    float m_titlePosX;
    float m_titleAnimSpeed;
    bool m_isTitleAnimFinished;

    // タスク内容フェードイン用
    int m_taskAlpha;
    float m_taskFadeSpeed;

    // アニメーション後の待機タイマー
    int m_animationWaitTimer;

    // ステップ移行の遅延タイマー
    int m_transitionDelayTimer;

    // パリィチュートリアル用一時停止制御
    bool m_hasShownParryTutorial; // パリィ説明を表示したかどうか
    bool m_isParryTutorialPaused; // パリィ説明表示中で停止しているか

    // 制限アクションフィードバック用
    int m_restrictedActionTimer;      // 表示タイマー
    AttackType m_restrictedActionType; // 制限されたアクションの種類
    int m_restrictedActionAlpha;      // フェード用アルファ値

    float m_prevScale; // 前回のスケール値
};