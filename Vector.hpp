#pragma once
#include <iostream>
#include "Function.hpp"

class vec2 {
public:
	float x;
	float y;
	vec2() :x(0), y(0) {}
	vec2(float val) :x(val), y(val) {}
	vec2(float xx, float yy) :x(xx), y(yy) {}

	vec2 operator+(const vec2& v) const {
		return vec2(v.x + x, v.y + y);
	}
	vec2 operator+(float r) const {
		return vec2(x + r, y + r);
	}
	vec2 operator-(const vec2& v) const {
		return vec2(x - v.x, y - v.y);
	}
	vec2 operator-() const {
		return vec2(-x, -y);
	}
	vec2 operator-(float r) const {
		return vec2(x - r, y - r);
	}
	vec2 operator*(const vec2& v) const {
		return vec2(x * v.x , y * v.y);
	}
	vec2 operator*(float r) const {
		return vec2(x * r, y * r);
	}
	vec2 operator/(float r) const {
		float k = 1.f / r;
		return vec2(x * k, y * k);
	}

	vec2& operator += (const vec2& v) {
		x += v.x;
		y += v.y;
		return *this;
	}
	vec2& operator -= (const vec2& v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	vec2 dot(const vec2& v) const {
		return *this * v;
	}
	float cross(const vec2& v) const {
		return x * v.y - y * v.x;
	}
	float norm() const {
		float r = sqrt(x * x + y * y);
		return r;
	}
	void normalize() {
		float rinv = mysqrtinv(x * x + y * y);
		x *= rinv;
		y *= rinv;
		return;
	}
	vec2 normalized() const {
		float rinv = mysqrtinv(x * x + y * y);
		return vec2(x * rinv, y * rinv);
	}
};

class vec3 {
public:
	float x;
	float y;
	float z;
	vec3() :x(0), y(0), z(0) {}
	vec3(float val) :x(val), y(val),z(val) {}
	vec3(float xx, float yy, float zz) :x(xx), y(yy), z(zz) {}

	vec3 operator+(const vec3& v) const {
		return vec3(x + v.x, y + v.y, z + v.z);
	}
	vec3 operator+(float r) const {
		return vec3(x + r, y + r, z + r);
	}
	vec3 operator-(const vec3& v) const {
		return vec3(x - v.x, y - v.y, z - v.z);
	} 
	vec3 operator-() const {
		return vec3(-x, -y, -z);
	}
	vec3 operator-(float r) const {
		return vec3(x - r, y - r, z - r);
	}
	//	cwiseProduct，不是dot
	vec3 operator*(const vec3& v) const {
		return vec3(x * v.x, y * v.y, z * v.z);
	}
	vec3 operator*(float r) const {
		return vec3(x * r, y * r, z * r);
	}
	vec3 operator/(float r) const {
		return vec3(x / r, y / r, z / r);
	}
	float operator[](unsigned index) const {
		return *(&x + index);
	}

	vec3& operator += (const vec3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}
	vec3& operator -= (const vec3& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}
	float dot(const vec3& v) const {
		return x * v.x + y * v.y + z * v.z;
	}
	vec3 cross(const vec3& v) const {
		return vec3(
			y * v.z - z * v.y,
			z * v.x - x * v.z,
			x * v.y - y * v.x);
	}
	float norm() const {
		return mysqrt(x * x + y * y + z * z);
	}
	void normalize() {
		float rinv = mysqrtinv(x * x + y * y + z * z);
		x *= rinv;
		y *= rinv;
		z *= rinv;
		return;
	}
	vec3 normalized() const {
		float rinv = mysqrtinv(x * x + y * y + z * z);
		return vec3(x * rinv, y * rinv, z * rinv);
	}

	static vec3 Min(const vec3& u, const vec3& v) {
		return vec3(
			std::min(u.x, v.x),
			std::min(u.y, v.y),
			std::min(u.z, v.z)
		);
	}
	static vec3 Max(const vec3& u, const vec3& v) {
		return vec3(
			std::max(u.x, v.x),
			std::max(u.y, v.y),
			std::max(u.z, v.z)
		);
	}
};

inline float dot(const vec2& u, const vec2& v) {
	return u.x * v.x + u.y * v.y;
}

inline float dot(const vec3& u, const vec3& v) {
	return u.x * v.x + u.y * v.y + u.z * v.z;
}

inline float cross(const vec2& u, const vec2& v) {
	return u.x * v.y - v.x * u.y;
}

inline vec3 cross(const vec3& u, const vec3& v) {
	return vec3(
		u.y * v.z - v.y * u.z,
		u.z * v.x - v.z * u.x,
		u.x * v.y - v.x * u.y
	);
}

inline vec3 lerp(const vec3& u, const vec3& v, float t) {
	return u * (1 - t) + v * t;
}
