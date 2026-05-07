#pragma once
#include "EffekseerForDXLib.h"
#include <vector>
#include "Stage.h"

class Player;

/// <summary>
/// アイテム基底クラス
/// </summary>
class ItemBase abstract
{
public:
	ItemBase() = default;
	virtual ~ItemBase() = default;

	virtual void Init()   abstract;
	virtual void Update(Player* player, const std::vector<Stage::StageCollisionData>& collisionData) abstract;
	virtual void Draw()   abstract;

	virtual bool IsUsed() const abstract;
	virtual bool IsExpired() const { return false; }

	// 位置設定用
	// 位置設定用
	virtual void SetPos(const VECTOR& pos) abstract;

	// デバッグ表示用
	static void SetDrawCollision(bool isDraw) { s_shouldDrawCollision = isDraw; }
	static bool ShouldDrawCollision() { return s_shouldDrawCollision; }

protected:
	virtual void DrawCollisionDebug() {}
	static bool s_shouldDrawCollision;
};