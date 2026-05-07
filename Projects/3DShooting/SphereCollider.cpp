#include "EffekseerForDXLib.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h" 
#include <cmath> 
#include <algorithm>

SphereCollider::SphereCollider(const VECTOR& center, float radius) 
    : m_center(center)
    , m_radius(radius)
{
}

// 当たり判定を行う
bool SphereCollider::IsIntersects(const Collider* other) const
{
	// 他のコライダーがnullptrの場合は交差しない
    if (!other) return false;

    // SphereCollider同士の判定
    const SphereCollider* sphere = dynamic_cast<const SphereCollider*>(other);
    if (sphere)
    {
		// 球同士の当たり判定
		float dx = m_center.x - sphere->m_center.x; // X座標の差
		float dy = m_center.y - sphere->m_center.y; // Y座標の差
		float dz = m_center.z - sphere->m_center.z; // Z座標の差
		float distSq = dx * dx + dy * dy + dz * dz; // 中心間の距離の二乗を計算
		float radiusSum = m_radius + sphere->m_radius; // 半径の和を計算

        // 球の中心間の距離の二乗が半径の和の二乗以下なら当たっている
		return distSq <= radiusSum * radiusSum; 
    }

    // CapsuleColliderとの判定
    const CapsuleCollider* capsule = dynamic_cast<const CapsuleCollider*>(other);
    if (capsule)
    {
        // 球とカプセルの当たり判定
        VECTOR capA = capsule->GetSegmentA();
        VECTOR capB = capsule->GetSegmentB();
        float capRadius = capsule->GetRadius();

		// カプセルの中心線分のベクトルを計算
        VECTOR ab = VSub(capB, capA);
        VECTOR ac = VSub(m_center, capA);

		float abLenSq = VDot(ab, ab); // 線分ABの長さの二乗
		float t = 0.0f; // 線分AB上の点へのパラメータt

		// 線分ABの長さがゼロに近い場合、球の中心がカプセルの端点に近いかどうかを判定
        if (abLenSq > 0.0f)
        {
			t = VDot(ac, ab) / abLenSq; // 球の中心から線分ABへの投影パラメータ
			t = (std::max)(0.0f, (std::min)(1.0f, t)); // tを0から1の範囲に制限
        }

		VECTOR closest = VAdd(capA, VScale(ab, t)); // 線分AB上の最も近い点を計算

		// 球の中心と線分AB上の最も近い点との距離を計算
        float distSq = VDot(VSub(m_center, closest), VSub(m_center, closest));
        float radiusSum = m_radius + capRadius;

        // 半径の和の二乗以下なら当たっている
		return distSq <= radiusSum * radiusSum; 
    }

    return false; // 未知のコライダータイプ
}

// Rayとの当たり判定を行う
bool SphereCollider::IsIsIntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHtPos, float& outHtDistSq) const
{
	// Rayの始点と終点をベクトルとして取得
    VECTOR rayDir = VSub(rayEnd, rayStart);
    float rayLengthSq = VDot(rayDir, rayDir);

    // Rayの長さが非常に短い、またはゼロの場合
    if (rayLengthSq < 0.0001f)
    {
        // Rayの始点と球の中心の距離を計算
        float distSq = VDot(VSub(m_center, rayStart), VSub(m_center, rayStart));
        // 始点が球内部にあればヒット
        if (distSq <= m_radius * m_radius)
        {
            outHtPos    = rayStart;
            outHtDistSq = 0.0f;
            return true;
        }
        return false;
    }

	// Rayの方向を正規化
    VECTOR oc = VSub(rayStart, m_center);
    float a = rayLengthSq;
    float b = 2.0f * VDot(oc, rayDir);
    float c = VDot(oc, oc) - m_radius * m_radius;
    float discriminant = b * b - 4 * a * c;

	// 判別式が負の場合、Rayと球は交差しない
    if (discriminant < 0)
    {
        return false; // 交差しない
    }
    else
    {
		float t  = (-b - std::sqrt(discriminant)) / (2.0f * a); // 最初の交点までのパラメータt
		float t1 = (-b + std::sqrt(discriminant)) / (2.0f * a); // もう一つの交点までのパラメータt

        // Rayの範囲(0から1)内で、最も近い交点を探す
        if (t < 0.0f || t > 1.0f) // 最初の交点が範囲外
        {
            t = t1; // 二つ目交点を試す

            // 二つ目の交点も範囲外
            if (t < 0.0f || t > 1.0f) 
            {
                return false; // どちらの交点もRayの範囲外
            }
        }

        outHtPos    = VAdd(rayStart, VScale(rayDir, t));
        outHtDistSq = VDot(VSub(outHtPos, rayStart), VSub(outHtPos, rayStart));
        return true; 
    } 
}