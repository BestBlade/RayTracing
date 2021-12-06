#pragma once
#include "Vector.hpp"
//	灯光类，没用到
class Light {
public:
	vec3 position;
	vec3 intensity;
	Light(const vec3& p, const vec3& i) :position(p), intensity(i) {}
	virtual ~Light() = default;
};
