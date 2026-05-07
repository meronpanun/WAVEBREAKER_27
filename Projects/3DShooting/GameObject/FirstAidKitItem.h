#pragma once
#include "ItemBase.h"
#include "EffekseerForDXLib.h"
#include "SphereCollider.h"
#include "Stage.h"

class Player;
class SphereCollider;

/// <summary>
/// 回復アイテムクラス
/// </summary>
class FirstAidKitItem : public ItemBase
{
public:
	FirstAidKitItem();
	virtual ~FirstAidKitItem();

	void Init() override;
	void Update(Player* player, const std::vector<Stage::StageCollisionData>& collisionData) override;
	void Draw() override;
	void DrawCollisionDebug() override;

	void SetPos(const VECTOR& pos) override { m_pos = pos; }
	bool IsUsed() const override { return m_isUsed; }
	bool IsExpired() const override { return m_lifeTimer <= 0; }

	static void LoadModel();
	static void DeleteModel();

private:
	VECTOR m_pos;

	SphereCollider m_collider;

	int m_modelHandle; // モデルハンドル

	float m_radius;    // 半径
	float m_velocityY; // 落下速度
	float m_rotY;      // Y軸回転角度

	bool m_isHit;      // プレイヤーと接触したかどうか
	bool m_isUsed;     // アイテムが使用されたかどうか
	bool m_isDropping; // 落下中かどうか
	int m_lifeTimer;

	static int s_modelHandle;
};
