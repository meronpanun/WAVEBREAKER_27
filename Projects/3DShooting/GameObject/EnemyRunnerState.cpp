#include "EnemyRunnerState.h"
#include "EnemyRunner.h"
#include "Player.h"
#include "Game.h"
#include "DxLib.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"

// =========================================================
// Run State
// =========================================================
void EnemyRunnerStateRun::Enter(EnemyRunner* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Run, true);
}

void EnemyRunnerStateRun::Update(EnemyRunner* enemy, const EnemyUpdateContext& context)
{
    if (!enemy->m_shouldUpdateAI) return;

    if (enemy->m_attackEndDelayTimer > 0)
    {
        enemy->m_attackEndDelayTimer -= enemy->m_aiUpdateInterval;
    }
    else
    {
        // 攻撃トリガー範囲内なら攻撃開始 (30.0f)
        if (enemy->CanAttackPlayer(context.player, 30.0f)) // EnemyRunnerConstants::kAttackTriggerRadius
        {
            enemy->m_hasAttackHit = false;
            enemy->ChangeState(std::make_shared<EnemyRunnerStateAttack>());
        }
    }
}

// =========================================================
// Attack State
// =========================================================
void EnemyRunnerStateAttack::Enter(EnemyRunner* enemy)
{
    enemy->m_hasAttackHit = false;
    enemy->ChangeAnimation(EnemyBase::AnimState::Attack, false);
}

void EnemyRunnerStateAttack::Update(EnemyRunner* enemy, const EnemyUpdateContext& context)
{
    if (!enemy->m_shouldUpdateAI) return;

    // 攻撃アニメーションの終了判定
    float currentAnimTotalTime = enemy->m_animationManager.GetAnimationTotalTime(enemy->m_modelHandle, "Armature|Attack");
    if (enemy->m_animTime > currentAnimTotalTime)
    {
        enemy->m_attackEndDelayTimer = 10; // EnemyRunnerConstants::kAttackEndDelay
        enemy->ChangeState(std::make_shared<EnemyRunnerStateRun>()); // 走り直す
    }
}

// =========================================================
// Dead State
// =========================================================
void EnemyRunnerStateDead::Enter(EnemyRunner* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Dead, false);
}

void EnemyRunnerStateDead::Update(EnemyRunner* enemy, const EnemyUpdateContext& context)
{
    // 何もしない
}
