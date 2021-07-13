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
		//Ĭ��û�з������䣬���겻���ڣ����ֲ����ڣ��������޴�û�в�������
		happened = false;
		coords = vec3(0.0f);
		normal = vec3(0.0f);
		distance = INT_MAX;
		m = NULL;
		obj = NULL;
	}
};