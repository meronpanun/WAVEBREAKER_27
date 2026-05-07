#pragma once
#include "DxLib.h"
#include <vector>
#include "Stage.h"

class EnemyBase;
class Camera;
class Effect;
class PlayerMovement;
class Player;

/// <summary>
/// プレイヤーのタックルアクションを管理するクラス
/// </summary>
class PlayerTackleSystem
{
public:
    PlayerTackleSystem();
    ~PlayerTackleSystem() = default;

    void Init(float cooldownMax, float speed, float damage);
    void Update(float deltaTime, bool isLockingOn, EnemyBase* lockedOnEnemy, 
                VECTOR& playerPos, PlayerMovement& movement, Camera* pCamera, 
                Effect* pEffect, const std::vector<EnemyBase*>& enemyList, 
                const std::vector<Stage::StageCollisionData>& collisionData,
                Player* pPlayer);

    /// <summary>
    /// クールダウンのみ更新（制限中用）
    /// </summary>
    void UpdateCooldownOnly(float deltaTime);

    // Getters
    bool IsTackling() const { return m_isTackling; }
    float GetCooldown() const { return m_cooldownTimer; }
    int GetTackleId() const { return m_tackleId; }
    float GetDamage() const { return m_damage; }
    VECTOR GetDir() const { return m_tackleDir; }

    // Settings
    void ResetCooldown() { m_cooldownTimer = 0.0f; }

private:
    bool m_isTackling;
    float m_cooldownTimer;
    float m_cooldownMax;
    float m_speed;
    float m_damage;
    float m_tackleFrame;
    int m_tackleId;
    int m_concentrationLineEffectHandle;
    int m_hitSECooldownTimer;
    VECTOR m_tackleDir;

    static constexpr int kTackleDuration = 35;
    static constexpr float kTackleStopMargin = 45.0f;
    static constexpr float kTackleFov = 100.0f;
    static constexpr float kTackleCameraZOffset = 30.0f;
    static constexpr float kConcentrationLineEffectZOffset = 15.0f;
};
