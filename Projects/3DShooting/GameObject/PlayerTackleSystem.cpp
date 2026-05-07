#include "PlayerTackleSystem.h"
#include "Player.h"
#include "EnemyBase.h"
#include "Camera.h"
#include "Effect.h"
#include "SoundManager.h"
#include "InputManager.h"
#include "Collision.h"
#include "CapsuleCollider.h"
#include "Game.h"

PlayerTackleSystem::PlayerTackleSystem()
    : m_isTackling(false)
    , m_cooldownTimer(0.0f)
    , m_cooldownMax(120.0f)
    , m_speed(20.0f)
    , m_damage(50.0f)
    , m_tackleFrame(0.0f)
    , m_tackleId(0)
    , m_concentrationLineEffectHandle(-1)
    , m_hitSECooldownTimer(0)
    , m_tackleDir(VGet(0, 0, 0))
{
}

void PlayerTackleSystem::Init(float cooldownMax, float speed, float damage)
{
    m_cooldownMax = cooldownMax;
    m_speed = speed;
    m_damage = damage;
    m_isTackling = false;
    m_cooldownTimer = 0.0f;
    m_tackleId = 0;
    m_concentrationLineEffectHandle = -1;
}

void PlayerTackleSystem::UpdateCooldownOnly(float deltaTime)
{
    if (m_hitSECooldownTimer > 0) m_hitSECooldownTimer--;
    if (m_cooldownTimer > 0) m_cooldownTimer -= 1.0f * Game::GetTimeScale();
}

void PlayerTackleSystem::Update(float deltaTime, bool isLockingOn, EnemyBase* lockedOnEnemy, 
                                VECTOR& playerPos, PlayerMovement& movement, Camera* pCamera, 
                                Effect* pEffect, const std::vector<EnemyBase*>& enemyList, 
                                const std::vector<Stage::StageCollisionData>& collisionData,
                                Player* pPlayer)
{
    UpdateCooldownOnly(deltaTime);

    // タックル開始判定
    if (isLockingOn && lockedOnEnemy && InputManager::GetInstance()->IsTriggerMouseLeft())
    {
        if (m_cooldownTimer <= 0)
        {
            m_isTackling = true;
            SoundManager::GetInstance()->Play("Player", "Tackle");
            m_tackleFrame = kTackleDuration;
            m_cooldownTimer = m_cooldownMax;
            m_tackleId++;
            m_tackleDir = VNorm(VSub(lockedOnEnemy->GetPos(), playerPos));

            if (pCamera)
            {
                pCamera->SetTargetFOV(kTackleFov * DX_PI_F / 180.0f);
                VECTOR offset = pCamera->GetOffset();
                offset.z = kTackleCameraZOffset;
                pCamera->SetOffset(offset);

                if (pEffect)
                {
                    VECTOR camPos = pCamera->GetPos();
                    m_concentrationLineEffectHandle = pEffect->PlayConcentrationLine(camPos.x, camPos.y, camPos.z);
                }
            }
        }
    }

    // タックル中の処理
    if (m_isTackling)
    {
        playerPos = VAdd(playerPos, VScale(m_tackleDir, m_speed * Game::GetTimeScale()));
        movement.SetPos(playerPos);

        // 地面制限
        if (playerPos.y < 0.0f) playerPos.y = 0.0f;

        // ステージ衝突判定
        CollisionResult res = Collision::CheckStageCollision(playerPos, 100.0f, 50.0f, 60.0f, collisionData);
        movement.SetPos(playerPos);

        // 敵との衝突・停止判定
        bool isBodyHit = false;
        VECTOR bodyCapA, bodyCapB;
        float bodyRadius;
        pPlayer->GetCapsuleInfo(bodyCapA, bodyCapB, bodyRadius);
        CapsuleCollider bodyCol(bodyCapA, bodyCapB, bodyRadius + kTackleStopMargin);

        Player::TackleInfo tackleInfo = pPlayer->GetTackleInfo();

        for (EnemyBase* enemy : enemyList)
        {
            if (!enemy || !enemy->IsAlive()) continue;

            auto enemyCollider = enemy->GetBodyCollider();
            if (enemyCollider && bodyCol.IsIntersects(enemyCollider.get()))
            {
                isBodyHit = true;
            }
        }

        m_tackleFrame -= 1.0f * Game::GetTimeScale();
        if (m_tackleFrame <= 0 || isBodyHit)
        {
            if (isBodyHit)
            {
                if (pCamera) pCamera->Shake(20.0f, 10);
                if (m_hitSECooldownTimer <= 0)
                {
                    SoundManager::GetInstance()->Play("Player", "TackleHit");
                    m_hitSECooldownTimer = 60;
                }
                VECTOR knockbackDir = VScale(m_tackleDir, -1.0f);
                movement.ApplyKnockback(VScale(knockbackDir, 15.0f));
            }

            m_isTackling = false;
            if (pCamera)
            {
                pCamera->ResetFOV();
                pCamera->ResetOffset();
            }
            if (m_concentrationLineEffectHandle != -1)
            {
                StopEffekseer3DEffect(m_concentrationLineEffectHandle);
                m_concentrationLineEffectHandle = -1;
            }
        }

        // 演出更新
        if (m_concentrationLineEffectHandle != -1 && pCamera)
        {
            VECTOR camPos = pCamera->GetPos();
            VECTOR camDir = VNorm(VSub(pCamera->GetTarget(), camPos));
            VECTOR effectPos = VAdd(camPos, VScale(camDir, kConcentrationLineEffectZOffset));
            SetPosPlayingEffekseer3DEffect(m_concentrationLineEffectHandle, effectPos.x, effectPos.y, effectPos.z);
            SetRotationPlayingEffekseer3DEffect(m_concentrationLineEffectHandle, -pCamera->GetPitch(), pCamera->GetYaw(), 0.0f);
        }
    }
}

