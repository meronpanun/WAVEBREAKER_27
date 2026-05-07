#include "EffekseerForDXLib.h"
#include "AmmoItem.h"
#include "CapsuleCollider.h"
#include "Collision.h"
#include "Stage.h"
#include "Player.h"
#include "DebugUtil.h"
#include <cassert>
#include <algorithm>
#include <cmath>
#include "SoundManager.h"

int AmmoItem::s_modelHandle = -1;

namespace
{
	constexpr int   kAmmoAmount    = 30;    // 回復する弾薬数
	constexpr float kInitialRadius = 20.0f; // 初期の半径
	constexpr float kDropGravity   = 0.5f;  // ドロップ時の重力加速度
	constexpr float kGroundY       = 0.0f;  // 地面のY座標
	constexpr float kRotateSpeed   = 0.05f; // 回転速度

	// 接地判定と衝突解決
	constexpr float kItemCollisionHeight = 10.0f;
	constexpr float kItemCollisionRadius = 60.0f; // 半径を大きくして埋まりを防ぐ
	constexpr float kItemCollisionYOffset = 25.0f; // オフセット調整
	constexpr int   kLifeTime = 1200;              // アイテムの寿命 (20秒)
	constexpr int   kFlashTime = 300;              // 点滅開始時間 (5秒)
}

AmmoItem::AmmoItem()
	: m_modelHandle(-1)
	, m_radius(kInitialRadius)
	, m_pos(VGet(0.0f, 0.0f, 0.0f))
	, m_collider(m_pos, m_radius)
	, m_isHit(false)
	, m_isUsed(false)
	, m_isDropping(true)
	, m_velocityY(0.0f)
	, m_rotY(0.0f)
	, m_lifeTimer(kLifeTime)
{
	// モデルの複製
	m_modelHandle = MV1DuplicateModel(s_modelHandle);
	assert(m_modelHandle != -1);
}

AmmoItem::~AmmoItem()
{
	// モデルの解放
	MV1DeleteModel(m_modelHandle);
}

void AmmoItem::LoadModel()
{
	s_modelHandle = MV1LoadModel("data/model/AmmoBox.mv1");
	assert(s_modelHandle != -1);
}

void AmmoItem::DeleteModel()
{
	MV1DeleteModel(s_modelHandle);
}

void AmmoItem::Init()
{
	m_isDropping = true;
	m_velocityY = 0.0f;
	m_lifeTimer = kLifeTime;
	m_collider.SetCenter(m_pos);
	m_collider.SetRadius(m_radius);
	MV1SetScale(m_modelHandle, VGet(3.0f, 3.0f, 3.0f));
}

void AmmoItem::Update(Player* player, const std::vector<Stage::StageCollisionData>& collisionData)
{
	if (IsUsed() || IsExpired()) return;

	// 寿命の更新
	if (m_lifeTimer > 0) m_lifeTimer--;

    // 重力適用
    m_velocityY -= kDropGravity;
    m_pos.y += m_velocityY;
    
    CollisionResult result = Collision::CheckStageCollision(m_pos, kItemCollisionHeight, kItemCollisionRadius, kItemCollisionYOffset, collisionData);

    // 地面（Y=0）またはステージ上に接地した場合
    if (result.isGrounded || m_pos.y <= kGroundY)
    {
        if (m_pos.y <= kGroundY) m_pos.y = kGroundY;
        
        // 接地したらバウンドなどをさせず、停止させる（必要に応じてバウンド処理を追加可能）
        if (m_velocityY < 0.0f)
        {
             m_velocityY = 0.0f;
             m_isDropping = false;
        }
    }
    
	// 常に回転させる
    m_rotY += kRotateSpeed;
    if (m_rotY > DX_TWO_PI) m_rotY -= DX_TWO_PI;

	m_collider.SetCenter(m_pos);
	m_collider.SetRadius(m_radius);

	const Collider* playerCollider = dynamic_cast<const Collider*>(player->GetBodyCollider().get());
	m_isHit = m_collider.IsIntersects(playerCollider);

	// 当たれば弾薬加算
	if (m_isHit)
	{
		SoundManager::GetInstance()->Play("Player", "AmmoItem"); // 弾薬アイテムSE再生
		// 現在の武器に応じて弾薬を加算
		switch (player->GetCurrentWeaponType())
		{
		case WeaponType::AssaultRifle:
			player->AddARAmmo(kAmmoAmount);
			break;
		case WeaponType::Shotgun:
			player->AddSGAmmo(kAmmoAmount);
			break;
		default:
			break;
		}
		m_isUsed = true;
	}
}

void AmmoItem::Draw()
{
	if (IsUsed() || IsExpired()) return;

	// 寿命が近い場合は点滅させる
	if (m_lifeTimer < kFlashTime)
	{
		if ((m_lifeTimer / 5) % 2 == 0) return;
	}

	MV1SetPosition(m_modelHandle, m_pos);
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, m_rotY, 0.0f));
	MV1DrawModel(m_modelHandle);

    if (s_shouldDrawCollision)
    {
        DrawCollisionDebug();
    }
}

void AmmoItem::DrawCollisionDebug()
{
    DebugUtil::DrawSphere(m_collider.GetCenter(), m_collider.GetRadius(), 16, 0xffff00);
}
