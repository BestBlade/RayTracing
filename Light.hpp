#pragma once

//灯光基类
class Light {
public:
	vec3 position;
	vec3 intensity;

	Light(const vec3& p, const vec3& i) {
		position = p;
		intensity = i;
	}

	virtual ~Light() = default;//虚析构，有子类继承
};