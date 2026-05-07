#pragma once
#include "TutorialTask.h"
#include "AttackType.h"

/// <summary>
/// 射撃タスク
/// </summary>
class ShootTutorialTask : public ITutorialTask
{
public:
    void Start(WaveManager* pWaveManager, Player* pPlayer) override;
    void Update() override;
    void DrawTaskUI(int x, int y, float scale, int alpha, const TaskTutorialManager* pManager) override;
    bool IsCompleted() const override;
    std::string GetTitle() const override { return "射撃訓練"; }
    float GetProgress() const override;
    std::string GetProgressText() const override;
    void NotifyEnemyKilled(int attackType) override;

private:
    int m_kills = 0;
    static constexpr int kGoal = 5;
};

/// <summary>
/// タックルタスク
/// </summary>
class TackleTutorialTask : public ITutorialTask
{
public:
    void Start(WaveManager* pWaveManager, Player* pPlayer) override;
    void Update() override;
    void DrawTaskUI(int x, int y, float scale, int alpha, const TaskTutorialManager* pManager) override;
    bool IsCompleted() const override;
    std::string GetTitle() const override { return "タックル訓練"; }
    float GetProgress() const override;
    std::string GetProgressText() const override;
    void NotifyEnemyKilled(int attackType) override;

private:
    int m_kills = 0;
    static constexpr int kGoal = 5;
};

/// <summary>
/// 盾投げタスク
/// </summary>
class ShieldThrowTutorialTask : public ITutorialTask
{
public:
    void Start(WaveManager* pWaveManager, Player* pPlayer) override;
    void Update() override;
    void DrawTaskUI(int x, int y, float scale, int alpha, const TaskTutorialManager* pManager) override;
    bool IsCompleted() const override;
    std::string GetTitle() const override { return "盾投げ訓練"; }
    float GetProgress() const override;
    std::string GetProgressText() const override;
    void NotifyEnemyKilled(int attackType) override;
    void NotifyShieldThrowKill() override;

private:
    int m_kills = 0;
    static constexpr int kGoal = 2;
};

/// <summary>
/// パリィタスク
/// </summary>
class ParryTutorialTask : public ITutorialTask
{
public:
    void Start(WaveManager* pWaveManager, Player* pPlayer) override;
    void Update() override;
    void DrawTaskUI(int x, int y, float scale, int alpha, const TaskTutorialManager* pManager) override;
    bool IsCompleted() const override;
    std::string GetTitle() const override { return "パリィ訓練"; }
    float GetProgress() const override;
    std::string GetProgressText() const override;
    void NotifyParrySuccess() override;

private:
    int m_parryCount = 0;
    static constexpr int kGoal = 3;
};
