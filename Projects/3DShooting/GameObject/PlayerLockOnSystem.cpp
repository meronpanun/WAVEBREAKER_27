#include "PlayerLockOnSystem.h"
#include "EnemyBase.h"
#include "Camera.h"
#include "Game.h"
#include "WaveManager.h"
#include "CollisionGrid.h"
#include "Collision.h"

PlayerLockOnSystem::PlayerLockOnSystem()
    : m_isLockingOn(false)
    , m_isTargetAvailable(false)
    , m_isAimingAtEnemy(false)
    , m_lockedOnEnemy(nullptr)
{
}

void PlayerLockOnSystem::Update(const VECTOR& playerPos, Camera* pCamera,  const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData, bool isGuarding, float tackleCooldown)
{
    if (!pCamera) return;

    VECTOR camPos = pCamera->GetPos();
    VECTOR camDir = VNorm(VSub(pCamera->GetTarget(), camPos));
    VECTOR rayEnd = VAdd(camPos, VScale(camDir, 2000.0f));

    m_isAimingAtEnemy = false;
    m_isTargetAvailable = false;

    // 近傍の敵を取得（グリッド利用）
    std::vector<EnemyBase*> nearbyEnemies;
    if (Game::m_pWaveManager)
    {
        Game::m_pWaveManager->GetCollisionGrid().GetNeighbors(playerPos, nearbyEnemies, false);
        Game::m_pWaveManager->GetCollisionGrid().GetNeighbors(VAdd(playerPos, VScale(camDir, 500.0f)), nearbyEnemies, false);
    }
    const std::vector<EnemyBase*>& targetEnemies = nearbyEnemies.empty() ? enemyList : nearbyEnemies;

    // 照準チェック
    for (const auto& enemy : targetEnemies)
    {
        if (!enemy || !enemy->IsAlive()) continue;

        VECTOR hitPos;
        float hitDistSq;
        EnemyBase::HitPart part = enemy->CheckHitPart(camPos, rayEnd, hitPos, hitDistSq);

        if (part == EnemyBase::HitPart::Body || part == EnemyBase::HitPart::Head)
        {
            if (CheckLineOfSight(camPos, hitPos, collisionData))
            {
                m_isAimingAtEnemy = true;
                break;
            }
        }
    }

    // ロックオン処理
    if (isGuarding && tackleCooldown <= 0)
    {
        m_isLockingOn = true;
        m_lockedOnEnemy = nullptr;
        float minScreenDistSq = -1.0f;

        for (EnemyBase* enemy : targetEnemies)
        {
            if (!enemy || !enemy->IsAlive()) continue;

            VECTOR diff = VSub(playerPos, enemy->GetPos());
            if (VSquareSize(diff) > kTackleMaxReachSq) continue;

            VECTOR enemyTargetPos = enemy->GetPos();
            enemyTargetPos.y += 70.0f;
            VECTOR toEnemyDir = VNorm(VSub(enemyTargetPos, camPos));

            if (VDot(camDir, toEnemyDir) > kLockOnAngleCos)
            {
                VECTOR screenPos = ConvWorldPosToScreenPos(enemyTargetPos);
                if (screenPos.z > 0)
                {
                    float dx = screenPos.x - (Game::GetScreenWidth() * 0.5f);
                    float dy = screenPos.y - (Game::GetScreenHeight() * 0.5f);

                    if (fabs(dy) < kLockOnMaxScreenOffsetY)
                    {
                        if (CheckLineOfSight(camPos, enemyTargetPos, collisionData))
                        {
                            float distSq = dx * dx + dy * dy;
                            if (minScreenDistSq < 0 || distSq < minScreenDistSq)
                            {
                                minScreenDistSq = distSq;
                                m_lockedOnEnemy = enemy;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        m_isLockingOn = false;
        m_lockedOnEnemy = nullptr;
    }

    m_isTargetAvailable = (m_lockedOnEnemy != nullptr);
}

bool PlayerLockOnSystem::CheckLineOfSight(const VECTOR& start, const VECTOR& end, const std::vector<Stage::StageCollisionData>& collisionData) const
{
    for (const auto& col : collisionData)
    {
        HITRESULT_LINE result = HitCheck_Line_Triangle(start, end, col.v1, col.v2, col.v3);
        if (result.HitFlag)
        {
            return false;
        }
    }
    return true;
}
