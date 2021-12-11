#pragma once
#include "Light.hpp"
#include "Function.hpp"
//	面光源类，实际上没有用到
class AreaLight : public Light {
public:
	vec3 normal;
	vec3 u;
	vec3 v;

	AreaLight(const vec3& p, const vec3& i) :Light(p, i), normal(0, -1, 0), u(1, 0, 0), v(0, 0, 1) {}

	vec3 sample() {
		float du = getrandom();
		float dv = getrandom();
		return position + u * du + v * dv;
	}
};