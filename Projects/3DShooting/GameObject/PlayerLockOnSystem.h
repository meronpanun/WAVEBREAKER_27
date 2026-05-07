#pragma once
#include "DxLib.h"
#include <vector>
#include "Stage.h"

class EnemyBase;
class Camera;

/// <summary>
/// プレイヤーのロックオンシステムを管理するクラス
/// </summary>
class PlayerLockOnSystem
{
public:
    PlayerLockOnSystem();
    ~PlayerLockOnSystem() = default;

    void Update(const VECTOR& playerPos, Camera* pCamera, const std::vector<EnemyBase*>& enemyList, const std::vector<Stage::StageCollisionData>& collisionData, bool isGuarding, float tackleCooldown);

    // Getters
    bool IsLockingOn() const { return m_isLockingOn; }
    EnemyBase* GetLockedOnEnemy() const { return m_lockedOnEnemy; }
    bool IsTargetAvailable() const { return m_isTargetAvailable; }
    bool IsAimingAtEnemy() const { return m_isAimingAtEnemy; }

private:
    bool CheckLineOfSight(const VECTOR& start, const VECTOR& end, const std::vector<Stage::StageCollisionData>& collisionData) const;

private:
    bool m_isLockingOn;
    bool m_isTargetAvailable;
    bool m_isAimingAtEnemy;
    EnemyBase* m_lockedOnEnemy;

    static constexpr float kLockOnAngleCos = 0.966f; // cos(15度)
    static constexpr float kLockOnMaxScreenOffsetY = 100.0f;
    static constexpr float kTackleMaxReachSq = 1800.0f * 1800.0f;
};
