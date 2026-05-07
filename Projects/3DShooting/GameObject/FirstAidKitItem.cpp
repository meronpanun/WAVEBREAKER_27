#include "EffekseerForDXLib.h"
#include "FirstAidKitItem.h"
#include "CapsuleCollider.h"
#include "Collision.h"
#include "Stage.h"
#include "Player.h"
#include "DebugUtil.h"
#include <cassert>
#include <algorithm>
#include <cmath>
#include "SoundManager.h"

int FirstAidKitItem::s_modelHandle = -1;

namespace
{
	// 初期半径
	constexpr float kInitialRadius = 20.0f; 

	constexpr float kHealAmount  = 30.0f; // 回復量
	constexpr float kDropGravity = 0.5f;  // 落下重力加速度
	constexpr float kGroundY     = 0.0f;  // 地面の高さ

	// アイテム用のサイズとオフセット
	constexpr float kItemCollisionHeight = 10.0f;
	constexpr float kItemCollisionRadius = 60.0f; // 半径を大きくして埋まりを防ぐ
	constexpr float kItemCollisionYOffset = 25.0f; // オフセット調整

	const float kRotateSpeed = 0.05f; // 回転速度
	constexpr int   kLifeTime = 1200;              // アイテムの寿命 (20秒)
	constexpr int   kFlashTime = 300;              // 点滅開始時間 (5秒)
}

FirstAidKitItem::FirstAidKitItem():
	m_modelHandle(-1),
	m_radius(kInitialRadius),
	m_pos(VGet(0.0f, 0.0f, 0.0f)),
	m_collider(m_pos, m_radius),
	m_isHit(false),
	m_isUsed(false),
	m_isDropping(true),
	m_velocityY(0.0f),
	m_rotY(0.0f),
	m_lifeTimer(kLifeTime)
{
	// モデルの複製
	m_modelHandle = MV1DuplicateModel(s_modelHandle);
	assert(m_modelHandle != -1);
}

FirstAidKitItem::~FirstAidKitItem()
{
	// モデルの解放
	MV1DeleteModel(m_modelHandle);
}

void FirstAidKitItem::LoadModel()
{
	s_modelHandle = MV1LoadModel("data/model/FirstAidKit.mv1");
	assert(s_modelHandle != -1);
}

void FirstAidKitItem::DeleteModel()
{
	MV1DeleteModel(s_modelHandle);
}

void FirstAidKitItem::Init()
{
	m_collider.SetCenter(m_pos);
	m_collider.SetRadius(m_radius);
	m_lifeTimer = kLifeTime;

	// モデルのスケール調整
	MV1SetScale(m_modelHandle, VGet(0.5f, 0.5f, 0.5f));
}

void FirstAidKitItem::Update(Player* player, const std::vector<Stage::StageCollisionData>& collisionData)
{
	if (IsUsed() || IsExpired()) return;

	// 寿命の更新
	if (m_lifeTimer > 0) m_lifeTimer--;

	// ドロップ演出（落下処理）
    // 重力適用
    m_velocityY -= kDropGravity;
    m_pos.y += m_velocityY;
    
    CollisionResult result = Collision::CheckStageCollision(m_pos, kItemCollisionHeight, kItemCollisionRadius, kItemCollisionYOffset, collisionData);

    // 地面（Y=0）またはステージ上に接地した場合
    if (result.isGrounded || m_pos.y <= kGroundY)
    {
        if (m_pos.y <= kGroundY) m_pos.y = kGroundY;
        
        // 接地したらバウンドなどをさせず、停止させる
        if (m_velocityY < 0.0f)
        {
             m_velocityY = 0.0f;
             m_isDropping = false;
        }
    }

    // 常に回転させる

    m_rotY += kRotateSpeed;
    if (m_rotY > DX_TWO_PI) m_rotY -= DX_TWO_PI;

	// コライダーの位置を更新
	m_collider.SetCenter(m_pos);
	m_collider.SetRadius(m_radius);

	// プレイヤーのカプセルコライダーを取得
	const Collider* playerCollider = dynamic_cast<const Collider*>(player->GetBodyCollider().get());

	// オブジェクト指向的な当たり判定
	m_isHit = m_collider.IsIntersects(playerCollider);

	// プレイヤーの体力が満タンでなく、かつ当たっていれば回復
	if (m_isHit && player->GetHealth() < player->GetMaxHealth())
	{
		SoundManager::GetInstance()->Play("Player", "Recovery"); // 回復SE再生
		player->AddHp(kHealAmount);
		m_isUsed = true;
	}
}

void FirstAidKitItem::Draw()
{
	if (IsUsed() || IsExpired()) return;

	// 寿命が近い場合は点滅させる
	if (m_lifeTimer < kFlashTime)
	{
		if ((m_lifeTimer / 5) % 2 == 0) return;
	}

	// モデルを描画
	MV1SetPosition(m_modelHandle, m_pos);
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, m_rotY, 0.0f));
	MV1DrawModel(m_modelHandle);

    if (s_shouldDrawCollision)
    {
        DrawCollisionDebug();
    }
}

void FirstAidKitItem::DrawCollisionDebug()
{
    DebugUtil::DrawSphere(m_collider.GetCenter(), m_collider.GetRadius(), 16, 0xffff00);
}