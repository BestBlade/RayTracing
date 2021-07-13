#pragma once
#include "Function.hpp"
#include "Material.hpp"
#include "Object.hpp"

class Object;

struct Intersection {
	bool happened;
	vec3 coords;
	vec3 normal;
	vec3 emit;
	float distance;
	Object* obj;
	Material* m;

	Intersection() {
		//默认没有发生反射，坐标不存在，发现不存在，距离无限大，没有材质属性
		happened = false;
		coords = vec3(0.0f);
		normal = vec3(0.0f);
		distance = INT_MAX;
		m = NULL;
		obj = NULL;
	}
};