#include "EnemyAcidState.h"
#include "EnemyAcid.h"
#include "Player.h"
#include "Game.h"
#include "DxLib.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"

// =========================================================
// Walk State
// =========================================================
void EnemyAcidStateWalk::Enter(EnemyAcid* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Walk, true);
}

void EnemyAcidStateWalk::Update(EnemyAcid* enemy, const EnemyUpdateContext& context)
{
    // 移動と遷移はEnemyAcid::UpdateState内で共通処理として実行されるため、
    // ここではアニメーション時間の更新を行うのみ
}

// =========================================================
// Back State
// =========================================================
void EnemyAcidStateBack::Enter(EnemyAcid* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Back, true);
}

void EnemyAcidStateBack::Update(EnemyAcid* enemy, const EnemyUpdateContext& context)
{
    // 移動と遷移は共通処理
}

// =========================================================
// Attack State
// =========================================================
void EnemyAcidStateAttack::Enter(EnemyAcid* enemy)
{
    enemy->m_hasAttacked = false;
    enemy->m_attackCooldown = 160; // EnemyAcidConstants::kAttackCooldownMax;
    enemy->ChangeAnimation(EnemyBase::AnimState::Attack, false);
}

void EnemyAcidStateAttack::Update(EnemyAcid* enemy, const EnemyUpdateContext& context)
{
    // 攻撃の弾出し等はEnemyAcid::UpdateStateで共通化されている
}

// =========================================================
// Stunned State
// =========================================================
void EnemyAcidStateStunned::Enter(EnemyAcid* enemy)
{
    enemy->m_isStunned = true;
    enemy->m_stunTimer = 120; // EnemyAcidConstants::kStunDuration
    enemy->ChangeAnimation(EnemyBase::AnimState::Dead, false); // 死亡アニメーションを流用
}

void EnemyAcidStateStunned::Update(EnemyAcid* enemy, const EnemyUpdateContext& context)
{
    // 怯み処理はUpdateState内で優先的に処理される
}

// =========================================================
// Dead State
// =========================================================
void EnemyAcidStateDead::Enter(EnemyAcid* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Dead, false);
    enemy->m_isDeadAnimPlaying = true;
    enemy->m_animTime = 0.0f;
}

void EnemyAcidStateDead::Update(EnemyAcid* enemy, const EnemyUpdateContext& context)
{
    // 死亡時は何もしない
}
