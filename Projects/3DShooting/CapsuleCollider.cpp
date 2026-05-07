#include "EffekseerForDXLib.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"
#include <algorithm>
#include <cmath>
#include <float.h>

// 線分pqと点cの間の最短距離を計算し、線分pq上の最短点も返す
VECTOR ClosestPtPointSegment(const VECTOR& c, const VECTOR& p, const VECTOR& q)
{
	VECTOR ab = VSub(q, p); // 線分pqのベクトル
	float   t = VDot(VSub(c, p), ab) / VDot(ab, ab); // 線分pq上の点cからの投影パラメータ

	return VAdd(p, VScale(ab, std::clamp(t, 0.0f, 1.0f))); // 線分pq上の最短点を返す
}

// Rayと球体の交差判定
bool IsIsIntersectsRaySphere(const VECTOR& rayStart, const VECTOR& rayEnd, const VECTOR& sphereCenter, float sphereRadius, VECTOR& outHtPos, float& outHtDistSq)
{
	VECTOR rayDir      = VSub(rayEnd, rayStart); // Rayの方向ベクトル
	float  rayLengthSq = VDot(rayDir, rayDir);   // Rayの長さの二乗

	// Rayの長さが0に近い場合、点と球体の交差判定を行う
	if (rayLengthSq < 0.0001f)
    {
		float distSq = VDot(VSub(sphereCenter, rayStart), VSub(sphereCenter, rayStart)); // 球心とRay始点の距離の二乗

		// 球心とRay始点の距離が球の半径以下なら、Rayは球に接触している
        if (distSq <= sphereRadius * sphereRadius)
        {
			outHtPos    = rayStart; // 交点はRayの始点
			outHtDistSq = 0.0f;     // 交点までの距離の二乗は0
            return true;
        }
        return false;
    }
	VECTOR oc = VSub(rayStart, sphereCenter); // Ray始点から球心へのベクトル

	float a = rayLengthSq;                                // Rayの長さの二乗
	float b = 2.0f * VDot(oc, rayDir);                    // Ray始点から球心へのベクトルとRayの方向ベクトルの内積の2倍
	float c = VDot(oc, oc) - sphereRadius * sphereRadius; // 球心とRay始点の距離の二乗から球の半径の二乗を引いた値
	float discriminant = b * b - 4 * a * c;               // 判別式

	// 判別式が負の場合、Rayと球は交差しない
    if (discriminant < 0)
    {
        return false;
    }
    else
    {
		float t  = (-b - std::sqrt(discriminant)) / (2.0f * a); // Rayと球の交点までのパラメータt
		float t1 = (-b + std::sqrt(discriminant)) / (2.0f * a); // もう一つの交点までのパラメータt

        // Rayの範囲(0から1)内で、最も近い交点を探す
        if (t < 0.0f || t > 1.0f)
        {
            t = t1;

			// もしt1も範囲外なら、交差しない
            if (t < 0.0f || t > 1.0f)
            {
                return false;
            }
        }

		outHtPos    = VAdd(rayStart, VScale(rayDir, t)); // 交点の位置を計算
		outHtDistSq = VDot(VSub(outHtPos, rayStart), VSub(outHtPos, rayStart)); // 交点までの距離の二乗を計算

        return true;
    }
}


CapsuleCollider::CapsuleCollider(const VECTOR& segmentA, const VECTOR& segmentB, float radius) : 
    m_segmentA(segmentA), 
    m_segmentB(segmentB), 
    m_radius(radius)
{
}

CapsuleCollider::~CapsuleCollider()
{
}

// CapsuleCollider同士の交差判定
bool CapsuleCollider::IsIntersects(const Collider* other) const
{
	// 他のコライダーがnullptrの場合は交差しない
	if (!other) return false; 

    // CapsuleCollider同士の判定
    const CapsuleCollider* capsule = dynamic_cast<const CapsuleCollider*>(other);
    if (capsule)
    {
        // 2つの線分間の最短距離を求めるロジック
		VECTOR d1 = VSub(m_segmentB, m_segmentA);
		VECTOR d2 = VSub(capsule->m_segmentB, capsule->m_segmentA);
		VECTOR r  = VSub(m_segmentA, capsule->m_segmentA); 

		// d1とd2の長さを計算
        float a = VDot(d1, d1);
        float e = VDot(d2, d2);
        float f = VDot(d2, r); 

		// d1とd2の内積を計算
        float s = 0.0f, t = 0.0f;
        float c = VDot(d1, r);
        float b = VDot(d1, d2);
        float denom = a * e - b * b;

		// 分母が0でない場合、sを計算
        if (denom != 0.0f)
        {
            s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
        }
        else 
        {
            s = 0.0f;
        }
		// 分母が0でない場合、tを計算
        t = (b * s + f) / e;

		// tが範囲外の場合、sを調整
        if (t < 0.0f) 
        {
            t = 0.0f;
            s = std::clamp((c) / a, 0.0f, 1.0f);
        }
        else if (t > 1.0f)
        {
            t = 1.0f;
            s = std::clamp((c + b) / a, 0.0f, 1.0f);
        }

		// 最短点を計算
        VECTOR p1 = VAdd(m_segmentA, VScale(d1, s));
        VECTOR p2 = VAdd(capsule->m_segmentA, VScale(d2, t));

		// 最短点間の距離を計算
        float distSq    = VDot(VSub(p1, p2), VSub(p1, p2));
        float radiusSum = m_radius + capsule->m_radius;

		// 半径の和の二乗と最短点間の距離の二乗を比較
        return distSq <= radiusSum * radiusSum;
    }

    // SphereColliderとの判定(CapsuleCollider側からSphereColliderを判定する)
    const SphereCollider* sphere = dynamic_cast<const SphereCollider*>(other);
    if (sphere)
    {
        // SphereCollider側で実装しているので、そちらを呼び出す
        return sphere->IsIntersects(this);
    }

    return false; // 未知のコライダータイプ
}

// Rayとカプセルの交差判定
bool CapsuleCollider::IsIsIntersectsRay(const VECTOR& rayStart, const VECTOR& rayEnd, VECTOR& outHitPos, float& outHitDistSq) const
{
	// Rayの始点と終点をベクトルとして取得
    VECTOR rayDir      = VSub(rayEnd, rayStart); 
    float  rayLengthSq = VDot(rayDir, rayDir);

    // Rayの長さが0に近い場合、点とカプセルの交差判定を行う
    if (rayLengthSq < 0.0001f)
    {
        VECTOR closestPointOnCapsuleSegment = ClosestPtPointSegment(rayStart, m_segmentA, m_segmentB);
        float distSq = VDot(VSub(rayStart, closestPointOnCapsuleSegment), VSub(rayStart, closestPointOnCapsuleSegment));

		// Ray始点とカプセルの線分上の最短点の距離がカプセルの半径以下なら、Rayはカプセルに接触している
        if (distSq <= m_radius * m_radius)
        {
			outHitPos    = rayStart; // 交点はRayの始点
			outHitDistSq = 0.0f;     // 交点までの距離の二乗は0
            return true;
        }
        return false;
    }

    float minT = FLT_MAX; // Rayのパラメータtの最小値
    bool hit = false;
    VECTOR currentHitPos;
    float currentHitDistSq;

    // カプセルの円柱部分とRayの交差判定
    // カプセルを線分ABと半径rの無限長円柱として扱う
    VECTOR OA = VSub(rayStart, m_segmentA);   // Ray始点からカプセル軸Aへのベクトル
    VECTOR AB = VSub(m_segmentB, m_segmentA); // カプセル軸のベクトル

    float abLenSq = VDot(AB, AB);

    // カプセルが球になっている場合(線分ABの長さがゼロに近い)
    if (abLenSq < 0.0001f) 
    {
        return IsIsIntersectsRaySphere(rayStart, rayEnd, m_segmentA, m_radius, outHitPos, outHitDistSq);
    }

    VECTOR u = VNorm(AB); // カプセル軸の単位ベクトル

    // Rayをカプセル軸に直交する平面に投影したベクトルを考える
    VECTOR v = VSub(rayDir, VScale(u, VDot(rayDir, u)));
    VECTOR w = VSub(OA, VScale(u, VDot(OA, u)));

	// 円柱の方程式に基づいて、Rayと円柱の交差判定を行う
    float a = VDot(v, v);
    float b = 2.0f * VDot(v, w);
    float c = VDot(w, w) - m_radius * m_radius;
    float discriminant = b * b - 4 * a * c;

	// 判別式が0以上の場合、Rayと円柱は交差する
    if (discriminant >= 0)
    {
		// 交差点のtを計算
        float sqrtDiscr = std::sqrt(discriminant);
        float tCyl0 = (-b - sqrtDiscr) / (2.0f * a);
        float tCyl1 = (-b + sqrtDiscr) / (2.0f * a);

		// 交差点のtがRayの範囲(0から1)内にあるか確認
        for (float tCyl : {tCyl0, tCyl1})
        {
            if (tCyl >= 0.0f && tCyl <= 1.0f) 
            {
				// 交差点の位置を計算
                VECTOR pIntersect  = VAdd(rayStart, VScale(rayDir, tCyl)); 
                VECTOR projOnAxis  = VAdd(m_segmentA, VScale(u, VDot(VSub(pIntersect, m_segmentA), u)));

				float t_axis = VDot(VSub(projOnAxis, m_segmentA), AB) / abLenSq; // 線分AB上の交差点のパラメータt

				// 交差点が線分AB上にあるか確認
                if (t_axis >= 0.0f && t_axis <= 1.0f)
                {
                    currentHitPos    = pIntersect;
                    currentHitDistSq = VDot(VSub(currentHitPos, rayStart), VSub(currentHitPos, rayStart));

                    // 交差点の距離が最小値より小さい場合
					if (currentHitDistSq < minT) 
                    {
						// 最小の交差距離を更新
                        minT = currentHitDistSq;

						// 交差点の位置と距離を出力
                        outHitPos    = currentHitPos; 
                        outHitDistSq = currentHitDistSq;
                        hit = true;
                    }
                }
            }
        }
    }

    // カプセルの両端の半球とRayの交差判定
    // RayとA端の球の交差
    if (IsIsIntersectsRaySphere(rayStart, rayEnd, m_segmentA, m_radius, currentHitPos, currentHitDistSq))
    {
		// 交差点の距離が最小値より小さい場合、更新
        if (currentHitDistSq < minT)
        {
			// 最小の交差距離を更新
            minT = currentHitDistSq;

			// 交差点の位置と距離を出力
            outHitPos    = currentHitPos;
            outHitDistSq = currentHitDistSq;
            hit = true;
        }
    }

    // RayとB端の球の交差
    if (IsIsIntersectsRaySphere(rayStart, rayEnd, m_segmentB, m_radius, currentHitPos, currentHitDistSq))
    {
		// 交差点の距離が最小値より小さい場合、更新
        if (currentHitDistSq < minT)
        {
			// 最小の交差距離を更新
            minT = currentHitDistSq;

			// 交差点の位置と距離を出力
            outHitPos    = currentHitPos;
            outHitDistSq = currentHitDistSq;
            hit = true;
        }
    }

    return hit; // 最終的な衝突結果を返す
}