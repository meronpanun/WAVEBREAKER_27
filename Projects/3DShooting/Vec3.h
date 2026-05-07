#pragma once
#include "EffekseerForDXLib.h"
#include <cmath>

/// <summary>
/// 3次元ベクトルクラス
/// </summary>
class Vec3
{
public:
	float x;
	float y;
	float z; 

public:
	// デフォルトコンストラクタ
	Vec3()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f; 
	}

	// 引数付きコンストラクタ
	Vec3(float posX, float posY, float posZ)
	{
		x = posX;
		y = posY;
		z = posZ;
	}

	// VECTORから変換するコンストラクタ
	Vec3(const VECTOR& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	// 座標設定関数
	void SetPos(float a, float b, float c)
	{
		x = a;
		y = b;
		z = c;
	}

	// 加算代入演算子(Vec3 = (Vec3 += Vec3))
	Vec3 operator+=(const Vec3& vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}

	// 加算演算子(Vec3 = Vec3 + Vec3)
	Vec3 operator+(const Vec3& vec) const
	{
		Vec3 temp{ x + vec.x, y + vec.y, z + vec.z };
		return temp;
	}

	// 減算代入演算子(Vec3 = (Vec3 -= Vec3))
	Vec3 operator-=(const Vec3& vec)
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}

	// 減算演算子(Vec3 = Vec3 - Vec3)
	Vec3 operator-(const Vec3& vec) const
	{
		Vec3 temp{ x - vec.x, y - vec.y, z - vec.z };
		return temp;
	}

	// スカラ乗算代入演算子(Vec3 = (Vec3 *= float))
	Vec3 operator*=(float scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
		return *this;
	}

	// スカラ乗算演算子(Vec3 = Vec3 * float)
	Vec3 operator*(float scale) const
	{
		Vec3 temp{ x * scale, y * scale, z * scale };
		return temp;
	}

	// スカラ除算代入演算子(Vec3 = (Vec3 /= float))
	Vec3 operator/=(float scale)
	{
		x /= scale;
		y /= scale;
		z /= scale; 
		return *this;
	}

	// スカラ除算演算子(Vec3 = Vec3 / float)
	Vec3 operator/(float scale) const
	{
		Vec3 temp{ x / scale, y / scale, z / scale };
		return temp;
	}

	// 長さの取得
	float Length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	// 正規化
	Vec3 Normalize() const
	{
		float len = Length(); // 長さを取得
		if (len == 0)
		{
			return *this; // 長さが0の場合はそのまま返す
		}
		return (*this) / len; // 自身の成分を長さで割って正規化
	}

    // DxライブラリのVECTORにキャストする
	VECTOR ToDxVECTOR() const 
	{
		VECTOR dxVec;

		dxVec.x = x;
		dxVec.y = y;
		dxVec.z = z;

		return dxVec;
	}

};

