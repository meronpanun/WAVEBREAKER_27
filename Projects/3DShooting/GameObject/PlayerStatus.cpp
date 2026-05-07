#include "PlayerStatus.h"
#include <algorithm>

PlayerStatus::PlayerStatus()
    : m_health(100.0f)
    , m_maxHealth(100.0f)
    , m_healthBarAnim(100.0f)
    , m_lowHealthBlinkTimer(0.0f)
    , m_deathTimer(0.0f)
    , m_isDead(false)
    , m_isInvincible(false)
    , m_isLowHealth(false)
{
}

void PlayerStatus::Init(float maxHp)
{
    m_maxHealth = maxHp;
    m_health = maxHp;
    m_healthBarAnim = maxHp;
    m_isDead = false;
    m_isInvincible = false;
    m_isLowHealth = false;
    m_lowHealthBlinkTimer = 0.0f;
    m_deathTimer = 0.0f;
}

void PlayerStatus::Update(float deltaTime)
{
    if (m_isDead)
    {
        m_deathTimer += deltaTime;
        m_healthBarAnim = 0.0f;
        return;
    }

    // HPバーアニメーション
    if (m_healthBarAnim != m_health)
    {
        if (m_healthBarAnim > m_health)
        {
            m_healthBarAnim -= kHpBarAnimSpeed;
            if (m_healthBarAnim < m_health) m_healthBarAnim = m_health;
        }
        else
        {
            m_healthBarAnim += kHpBarAnimSpeed;
            if (m_healthBarAnim > m_health) m_healthBarAnim = m_health;
        }
    }

    // 体力低下警告
    if (m_health <= kLowHealthThreshold)
    {
        m_isLowHealth = true;
        m_lowHealthBlinkTimer += deltaTime;
    }
    else
    {
        m_isLowHealth = false;
        m_lowHealthBlinkTimer = 0.0f;
    }
}

void PlayerStatus::SetHealth(float health)
{
    m_health = health;
}

void PlayerStatus::AddHp(float value)
{
    m_health = (std::min)(m_health + value, m_maxHealth);
}

void PlayerStatus::TakeDamage(float damage)
{
    if (m_isInvincible || m_isDead) return;

    m_health -= damage;
    if (m_health <= 0.0f)
    {
        m_health = 0.0f;
        m_isDead = true;
    }
}
