#include "EffekseerForDXLib.h"
#pragma once

/// <summary>
/// 当たり判定関数
/// </summary>
class Collider abstract
{
public:
	Collider() = default;
	virtual ~Collider() = default;

	// 当たり判定関数
	virtual bool IsIntersects(const Collider* other) const abstract;

	/// <summary>
	/// Rayとの当たり判定を行う
	/// </summary>
	/// <param name="rayStart">Rayの始点</param>
	/// <param name="rayEnd">Rayの終点</param>
	/// <param name="outHtPos">当たった位置</param>
	/// <param name="outHtDistSq">当たった位置までの距離の二乗</param>
	/// <returns></returns>
	virtual bool IsIsIntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const abstract;
};

