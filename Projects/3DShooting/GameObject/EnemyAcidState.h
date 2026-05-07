#pragma once
#include "EnemyState.h"

class EnemyAcid;

// ---------------------------------------------------------
// 徘徊・追跡状態
// ---------------------------------------------------------
class EnemyAcidStateWalk : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override;
};

// ---------------------------------------------------------
// 後退状態
// ---------------------------------------------------------
class EnemyAcidStateBack : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override;
};

// ---------------------------------------------------------
// 攻撃状態
// ---------------------------------------------------------
class EnemyAcidStateAttack : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override;
};

// ---------------------------------------------------------
// 怯み（スタン）状態
// ---------------------------------------------------------
class EnemyAcidStateStunned : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override;
};

// ---------------------------------------------------------
// 死亡状態
// ---------------------------------------------------------
class EnemyAcidStateDead : public EnemyState<EnemyAcid>
{
public:
    void Enter(EnemyAcid* enemy) override;
    void Update(EnemyAcid* enemy, const EnemyUpdateContext& context) override;
};
