#pragma once
#include "Collider.h"

/// <summary>
/// 球体コライダークラス
/// </summary>
class SphereCollider : public Collider
{
public:
    SphereCollider(const VECTOR& center = VGet(0, 0, 0), float radius = 1.0f);
    virtual ~SphereCollider() = default;

    /// <summary>
	/// 当たり判定を行う
    /// </summary>
	/// <param name="other">他のコライダー</param>
	/// <returns>true: 当たっている, false: 当たっていない</returns>
    bool IsIntersects(const Collider* other) const override;

    /// <summary>
	/// Rayとの当たり判定を行う
    /// </summary>
	/// <param name="rayStart">Rayの始点</param>
	/// <param name="rayEnd">Rayの終点</param>
	/// <param name="outHtPos">当たった位置</param>
	/// <param name="outHtDistSq">当たった位置までの距離の二乗</param>
    /// <returns></returns>
    bool IsIsIntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHitPos, float& outHitDistSq) const override;

    /// <summary>
	/// 球の中心と半径を取得する
    /// </summary>
	/// <returns>球の中心と半径</returns>
    VECTOR GetCenter() const { return m_center; }

    /// <summary>
	/// 球の半径を取得する
    /// </summary>
	/// <returns>球の半径</returns>
    float GetRadius() const { return m_radius; }

    /// <summary>
	/// 球の中心と半径を設定する
    /// </summary>
	/// <param name="center">球の中心</param>
    void SetCenter(const VECTOR& center) { m_center = center; }

    /// <summary>
	/// 球の半径を設定する  
    /// </summary>
	/// <param name="radius">球の半径</param>
    void SetRadius(float radius) { m_radius = radius; }

private:
	VECTOR m_center; // 球の中心

	float  m_radius; // 球の半径
};