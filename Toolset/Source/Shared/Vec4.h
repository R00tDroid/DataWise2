#pragma once

template<class T>
struct Vec4
{
	union
	{
		T f[4];
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };
	};

	Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
	Vec4(T x, T y, T z) : x(x), y(y), z(z), w(1) {}
	Vec4() : x(0), y(0), z(0), w(0) {}

	Vec4 operator * (Vec4& other)
	{
		return { x * other.x, y * other.y, z * other.z, w * other.w };
	}

	Vec4 operator * (const float& other)
	{
		return { x * other, y * other, z * other, w * other };
	}

	void operator *= (Vec4& other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;
	}

	Vec4 operator / (Vec4& other)
	{
		return { x / other.x, y / other.y, z / other.z, w / other.w };
	}

	void operator /= (Vec4& other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;
	}

	Vec4 operator + (const Vec4& other) const
	{
		return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	Vec4 operator - (const Vec4& other) const
	{
		return Vec4(x - other.x, y - other.y, z - other.z, w - other.w );
	}

	bool operator < (const Vec4& other) const
	{
		return x < other.x && y < other.y && z < other.z && w < other.w;
	}
};

typedef Vec4<float> Color;

namespace Colors
{
	static Color Black(0, 0, 0, 1);
}
