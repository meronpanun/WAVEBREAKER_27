#include "EnemyNormalState.h"
#include "EnemyNormal.h"
#include "Player.h"
#include "Game.h"
#include "DxLib.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"

// =========================================================
// Walk State
// =========================================================
void EnemyNormalStateWalk::Enter(EnemyNormal* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Walk, true);
}

void EnemyNormalStateWalk::Update(EnemyNormal* enemy, const EnemyUpdateContext& context)
{
    if (!enemy->m_shouldUpdateAI) return;

    // 攻撃が届くまでWalkを維持し、届いたらAttackに遷移
    if (enemy->CanAttackPlayer(context.player))
    {
        enemy->m_hasAttackHit = false;
        enemy->ChangeState(std::make_shared<EnemyNormalStateAttack>());
    }
}

// =========================================================
// Attack State
// =========================================================
void EnemyNormalStateAttack::Enter(EnemyNormal* enemy)
{
    enemy->m_hasAttackHit = false;
    enemy->ChangeAnimation(EnemyBase::AnimState::Attack, false);
}

void EnemyNormalStateAttack::Update(EnemyNormal* enemy, const EnemyUpdateContext& context)
{
    if (!enemy->m_shouldUpdateAI) return;

    // 攻撃アニメーションはループしないので、終了したらディレイタイマーをセット
    float currentAnimTotalTime = enemy->m_animationManager.GetAnimationTotalTime(enemy->m_modelHandle, "ATK");
    if (enemy->m_animTime > currentAnimTotalTime)
    {
        if (enemy->m_attackEndDelayTimer <= 0) 
        {
            enemy->m_attackEndDelayTimer = 20; // EnemyNormalConstants::kAttackEndDelay
        }
    }

    // ディレイタイマーが動作中ならカウントダウン
    if (enemy->m_attackEndDelayTimer > 0)
    {
        enemy->m_attackEndDelayTimer -= enemy->m_aiUpdateInterval; // 間引き分減算
        if (enemy->m_attackEndDelayTimer <= 0)
        {
            enemy->m_hasAttackHit = false; // 攻撃ヒットフラグをリセット
            
            std::shared_ptr<CapsuleCollider> playerBodyCollider = context.player.GetBodyCollider();
            bool isPlayerInAttackRange = enemy->m_pAttackRangeCollider->IsIntersects(playerBodyCollider.get());

            if (isPlayerInAttackRange)
            {
                // 攻撃範囲内なら再度攻撃
                enemy->ChangeState(std::make_shared<EnemyNormalStateAttack>());
            }
            else
            {
                // 範囲外なら歩行
                enemy->ChangeState(std::make_shared<EnemyNormalStateWalk>());
            }
        }
    }
}

// =========================================================
// Damage State
// =========================================================
void EnemyNormalStateDamage::Enter(EnemyNormal* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Walk, true); // ダメージ中は歩行モーションを流用
}

void EnemyNormalStateDamage::Update(EnemyNormal* enemy, const EnemyUpdateContext& context)
{
    // ダメージ状態の更新は毎フレーム行う
    if (enemy->m_damageTimer > 0)
    {
        enemy->m_damageTimer--;
        if (enemy->m_damageTimer <= 0)
        {
            enemy->ChangeState(std::make_shared<EnemyNormalStateWalk>()); // 復帰
        }
    }

    // ノックバック処理（少し後ろに下がる）
    VECTOR toPlayer = VSub(context.player.GetPos(), enemy->m_pos);
    toPlayer.y = 0.0f;
    if (VSquareSize(toPlayer) > 0.0001f) // EnemyNormalConstants::kPushBackEpsilon
    {
        VECTOR knockbackDir = VNorm(VScale(toPlayer, -1.0f));
        // 減衰させつつ移動
        if (enemy->m_damageTimer > 10) // 最初だけ下がる
        {
            float knockbackSpeed = 2.0f * Game::GetTimeScale();
            enemy->m_pos = VAdd(enemy->m_pos, VScale(knockbackDir, knockbackSpeed));
        }
    }
}

// =========================================================
// Dead State
// =========================================================
void EnemyNormalStateDead::Enter(EnemyNormal* enemy)
{
    enemy->ChangeAnimation(EnemyBase::AnimState::Dead, false);
}

void EnemyNormalStateDead::Update(EnemyNormal* enemy, const EnemyUpdateContext& context)
{
    // 死亡アニメーション中は移動や攻撃を行わない
}
