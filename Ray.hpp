#pragma once
#include "Function.hpp"

class Ray {
public:
	vec3 ori;
	vec3 dir, dir_inv;
	float t;
	float t_min, t_max;

	Ray(const vec3& ori_, const vec3& dir_, const double t_ = 0.0) :ori(ori_), dir(dir_), t(t_) {
		dir_inv = vec3(1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z);
		t_min = 0;
		t_max = INT_MAX;
	}

	vec3 operator()(float t) const {
		return ori + dir * t;
	}
};