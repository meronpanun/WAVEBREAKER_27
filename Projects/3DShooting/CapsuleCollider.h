#pragma once
#include "Collider.h"

/// <summary>
/// カプセル型コライダークラス
/// </summary>
class CapsuleCollider : public Collider
{
public:
    CapsuleCollider(const VECTOR& segmentA = VGet(0, 0, 0), const VECTOR& segmentB = VGet(0, 1, 0), float radius = 1.0f);
    virtual ~CapsuleCollider();

    /// <summary>
	/// 他のコライダーと当たっているかどうかを判定する
    /// </summary>
	/// <param name="other">他のコライダー</param>s
	/// <returns>trueなら当たっている</returns>
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
	/// カプセルのセグメントAを取得する
    /// </summary>
	/// <returns>セグメントA半径</returns>
    VECTOR GetSegmentA() const { return m_segmentA; }

    /// <summary>
	/// カプセルのセグメントBを取得する
    /// </summary>
	/// <returns>セグメントB半径</returns>
    VECTOR GetSegmentB() const { return m_segmentB; }

    /// <summary>
	/// カプセルの半径を取得する
    /// </summary>
	/// <returns>カプセルの半径</returns>
    float GetRadius() const { return m_radius; }

    /// <summary>
	/// カプセルのセグメントAとBを設定する
    /// </summary>
	/// <param name="a">セグメントA</param>
	/// <param name="b">セグメントB</param>
    void SetSegment(const VECTOR& a, const VECTOR& b) { m_segmentA = a; m_segmentB = b; }

    /// <summary>
	/// カプセルの半径を設定する
    /// </summary>
	/// <param name="radius">カプセルの半径</param>
    void SetRadius(float radius) { m_radius = radius; }

private:
	VECTOR m_segmentA; // カプセルのセグメントA
	VECTOR m_segmentB; // カプセルのセグメントB

	float  m_radius; // カプセルの半径
};