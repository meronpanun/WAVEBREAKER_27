#pragma once
#include <cmath>

/// <summary>
/// 2次元ベクトルクラス
/// </summary>
class Vec2
{
public:
	float x;
	float y;

public:
	Vec2()
	{
		x = 0.0f;
		y = 0.0f;
	}
	Vec2(float posX, float posY)
	{
		x = posX;
		y = posY;
	}
	void SetPos(float a, float b)
	{
		x = a;
		y = b;
	}

	// Vec2 = (Vec2 += Vec2)
	Vec2 operator+=(const Vec2& vec)
	{
		x += vec.x;
		y += vec.y;
		return *this;
	}
	// Vec2 = Vec2 + Vec2
	Vec2 operator+(const Vec2& vec) const
	{
		Vec2 temp{ x + vec.x, y + vec.y };
		return temp;
	}
	// Vec2 = (Vec2 -= Vec2)
	Vec2 operator-=(const Vec2& vec)
	{
		x -= vec.x;
		y -= vec.y;
		return *this;
	}
	// Vec2 = Vec2 -= Vec2
	Vec2 operator-(const Vec2& vec) const
	{
		Vec2 temp{ x - vec.x, y - vec.y };
		return temp;
	}
	// Vec2 = (Vec2 *= Vec2)
	Vec2 operator*=(float scale)
	{
		x *= scale;
		y *= scale;
		return *this;
	}
	// Vec2 = Vec2 * Vec2
	Vec2 operator*(float scale) const
	{
		Vec2 temp{ x * scale, y * scale };
		return temp;
	}
	// Vec2 = (Vec2 /= Vec2)
	Vec2 operator/=(float scale)
	{
		x /= scale;
		y /= scale;
		return *this;
	}
	// Vec2 = Vec2 / Vec2
	Vec2 operator/(float scale) const
	{
		Vec2 temp{ x / scale, y / scale };
		return temp;
	}

	// 長さの取得
	float Length()
	{
		return sqrtf(x * x + y * y);
	}

	// 正規化
	Vec2 Normalize()
	{
		float len = Length();
		if (len == 0)
		{
			return *this;
		}
		return (*this) / len;
	}
};

