#pragma once

#include<stdint.h>
#include<cmath>

struct vec2
{
public:
	float x;
	float y;

public:
	vec2() = default;
	constexpr vec2(const float x_, const float y_) : x(x_), y(y_) {};
public:
	inline constexpr vec2 operator+(const vec2& o) const {
		return vec2(x + o.x, y + o.y);
	}
	inline constexpr vec2 operator-(const vec2& o) const {
		return vec2(x - o.x, y - o.y);
	}
	inline constexpr vec2 operator/(const vec2& o) const {
		return vec2(x / o.x, y / o.y);
	}
	inline constexpr vec2 operator/(const float o) const {
		return vec2(x / o, y / o);
	}
	inline constexpr vec2 operator*(const vec2& o) const {
		return vec2(x * o.x, y * o.y);
	}
	inline constexpr vec2 operator*(const float o) const {
		return vec2(x * o, y * o);
	}
	inline constexpr vec2& operator+=(const vec2& o) {
		x += o.x;
		y += o.y;
		return *this;
	}
	inline constexpr float dot(const vec2& o) const {
		return x * o.x + y * o.y;
	}
	inline constexpr float sqLength() const {
		return this->dot(*this);
	}
	inline float length() const {
		return sqrt(this->sqLength());
	}
	inline constexpr float sqDistance(const vec2& o) const {
		vec2 d = *this - o;
		return d.sqLength();
	}
	inline float distance(const vec2& o) const {
		vec2 d = *this - o;
		return d.length();
	}
};

struct vec2i
{
	int32_t x, y;

public:
	inline constexpr vec2i(const int32_t x_, const int32_t y_) : x(x_), y(y_) {};

	inline constexpr vec2i operator+(const vec2i o) const {
		return vec2i(x + o.x, y + o.y);
	}
	inline constexpr vec2i operator-(const vec2i o) const {
		return vec2i(x - o.x, y - o.y);
	}
};

struct vec2ui
{
	uint32_t x, y;

public:
	inline constexpr vec2ui(const uint32_t x_, const uint32_t y_) : x(x_), y(y_) {};

	inline constexpr vec2ui operator+(const vec2ui o) const {
		return vec2ui(x + o.x, y + o.y);
	}
	inline constexpr vec2ui operator-(const vec2ui o) const {
		return vec2ui(x - o.x, y - o.y);
	}
};