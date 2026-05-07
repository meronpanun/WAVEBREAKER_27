#pragma once
#include "EnemyState.h"

class EnemyBoss;

// ---------------------------------------------------------
// 移動・追跡状態
// ---------------------------------------------------------
class EnemyBossStateWalk : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override;
};

// ---------------------------------------------------------
// 近接攻撃状態
// ---------------------------------------------------------
class EnemyBossStateAttack : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override;
};

// ---------------------------------------------------------
// 遠距離攻撃状態
// ---------------------------------------------------------
class EnemyBossStateLongRange : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override;
};

// ---------------------------------------------------------
// 怯み（スタン）状態
// ---------------------------------------------------------
class EnemyBossStateStunned : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override;
};

// ---------------------------------------------------------
// 死亡状態
// ---------------------------------------------------------
class EnemyBossStateDead : public EnemyState<EnemyBoss>
{
public:
    void Enter(EnemyBoss* enemy) override;
    void Update(EnemyBoss* enemy, const EnemyUpdateContext& context) override;
};
