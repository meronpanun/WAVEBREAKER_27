#pragma once
#include "DxLib.h"

/// <summary>
/// プレイヤーのステータス（体力、死亡状態など）を管理するクラス
/// </summary>
class PlayerStatus
{
public:
    PlayerStatus();
    ~PlayerStatus() = default;

    void Init(float maxHp);
    void Update(float deltaTime);

    // Getters
    float GetHealth() const { return m_health; }
    float GetMaxHealth() const { return m_maxHealth; }
    float GetHealthBarAnim() const { return m_healthBarAnim; }
    bool IsDead() const { return m_isDead; }
    bool IsInvincible() const { return m_isInvincible; }
    bool IsLowHealth() const { return m_isLowHealth; }
    float GetLowHealthBlinkTimer() const { return m_lowHealthBlinkTimer; }
    float GetDeathTimer() const { return m_deathTimer; }

    // Setters
    void SetHealth(float health);
    void SetInvincible(bool invincible) { m_isInvincible = invincible; }
    void AddHp(float value);
    void TakeDamage(float damage);
    void SetDead(bool dead) { m_isDead = dead; }

private:
    float m_health;
    float m_maxHealth;
    float m_healthBarAnim;
    float m_lowHealthBlinkTimer;
    float m_deathTimer;
    
    bool m_isDead;
    bool m_isInvincible;
    bool m_isLowHealth;

    static constexpr float kHpBarAnimSpeed = 1.5f;
    static constexpr float kLowHealthThreshold = 30.0f;
};
