#pragma once
#include "DxLib.h"
#include <string>

class Player;
class WaveManager;
class TaskTutorialManager;

/// <summary>
/// チュートリアルの個別タスクの基底インターフェース
/// </summary>
class ITutorialTask
{
public:
    virtual ~ITutorialTask() = default;

    // 初期化（開始時に一度呼ばれる）
    virtual void Start(WaveManager* pWaveManager, Player* pPlayer) = 0;
    
    // 更新処理
    virtual void Update() = 0;
    
    // タスク固有のUI描画
    virtual void DrawTaskUI(int x, int y, float scale, int alpha, const TaskTutorialManager* pManager) = 0;

    // 完了判定
    virtual bool IsCompleted() const = 0;

    // タイトル文字列の取得
    virtual std::string GetTitle() const = 0;

    // 進捗率（0.0 ~ 1.0）の取得
    virtual float GetProgress() const = 0;
    
    // 現在の進捗文字列（例: "3/5"）の取得
    virtual std::string GetProgressText() const = 0;

    // 通知系
    virtual void NotifyEnemyKilled(int attackType) {}
    virtual void NotifyShieldThrowKill() {}
    virtual void NotifyParrySuccess() {}
};
