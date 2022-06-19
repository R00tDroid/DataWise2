#pragma once

template<class T>
struct Vec2
{
	T x, y;

	Vec2(T x, T y) : x(x), y(y) {}
	Vec2() : x(0), y(0) {}

	Vec2 operator * (Vec2& other)
	{
		return { x * other.x, y * other.y };
	}

	void operator *= (Vec2& other)
	{
		x *= other.x;
		y *= other.y;
	}

	Vec2 operator / (Vec2& other)
	{
		return { x / other.x, y / other.y };
	}

	void operator /= (Vec2& other)
	{
		x /= other.x;
		y /= other.y;
	}

	void operator /= (const float& other)
	{
		x /= other;
		y /= other;
	}

	void operator += (const Vec2& other)
	{
		x += other.x;
		y += other.y;
	}

	void operator -= (const Vec2& other)
	{
		x -= other.x;
		y -= other.y;
	}

	bool operator < (const Vec2& other) const
	{
		return x < other.x || (x == other.x && y < other.y);
	}

	Vec2 operator + (const Vec2& other) const
	{
		return Vec2<T>(x + other.x, y + other.y);
	}

	Vec2 operator - (const Vec2& other) const
	{
		return Vec2<T>(x - other.x, y - other.y);
	}

	template<class N> Vec2<N> To()
	{
		return Vec2<N>((N)x, (N)y);
	}

	static float cross(Vec2& a, Vec2& b) {
		return (a.x * b.y) - (a.y * b.x);
	}
};

typedef Vec2<float> Float2;
typedef Vec2<int> Int2;