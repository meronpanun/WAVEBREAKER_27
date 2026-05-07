#pragma once
#include "EnemyState.h"

// 前方宣言
class EnemyRunner;

// ---------------------------------------------------------
// 走行状態（プレイヤーを追跡・回避）
// ---------------------------------------------------------
class EnemyRunnerStateRun : public EnemyState<EnemyRunner>
{
public:
    void Enter(EnemyRunner* enemy) override;
    void Update(EnemyRunner* enemy, const EnemyUpdateContext& context) override;
};

// ---------------------------------------------------------
// 攻撃状態
// ---------------------------------------------------------
class EnemyRunnerStateAttack : public EnemyState<EnemyRunner>
{
public:
    void Enter(EnemyRunner* enemy) override;
    void Update(EnemyRunner* enemy, const EnemyUpdateContext& context) override;
};

// ---------------------------------------------------------
// 死亡状態
// ---------------------------------------------------------
class EnemyRunnerStateDead : public EnemyState<EnemyRunner>
{
public:
    void Enter(EnemyRunner* enemy) override;
    void Update(EnemyRunner* enemy, const EnemyUpdateContext& context) override;
};
