#pragma once
#include "EnemyState.h"

class EnemyNormal;

// ---------------------------------------------------------
// 通常状態（プレイヤーを追跡・徘徊）
// ---------------------------------------------------------
class EnemyNormalStateWalk : public EnemyState<EnemyNormal>
{
public:
    void Enter(EnemyNormal* enemy) override;
    void Update(EnemyNormal* enemy, const EnemyUpdateContext& context) override;
};

// ---------------------------------------------------------
// 攻撃状態
// ---------------------------------------------------------
class EnemyNormalStateAttack : public EnemyState<EnemyNormal>
{
public:
    void Enter(EnemyNormal* enemy) override;
    void Update(EnemyNormal* enemy, const EnemyUpdateContext& context) override;
};

// ---------------------------------------------------------
// ダメージ（怯み）状態
// ---------------------------------------------------------
class EnemyNormalStateDamage : public EnemyState<EnemyNormal>
{
public:
    void Enter(EnemyNormal* enemy) override;
    void Update(EnemyNormal* enemy, const EnemyUpdateContext& context) override;
};

// ---------------------------------------------------------
// 死亡状態
// ---------------------------------------------------------
class EnemyNormalStateDead : public EnemyState<EnemyNormal>
{
public:
    void Enter(EnemyNormal* enemy) override;
    void Update(EnemyNormal* enemy, const EnemyUpdateContext& context) override;
};
