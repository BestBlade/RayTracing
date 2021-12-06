#pragma once
#include "Vector.hpp"
#include "Material.hpp"
//	声明Object类，避免头文件交叉调用
class Object;
//	交点类
class Intersection {
public:
	bool happened;	//	相交是否发生
	float distance;	//	相交距离
	Object* obj;	//	相交点物体属性
	Material* m;	//	相交点材质

	vec3 coords;	//	相交点坐标
	vec3 normal;	//	相交点法向量
	vec3 emit;		//	相交点发光属性
	//	默认构造函数
	Intersection() {
		happened = false;	//	相交未发生
		distance = INT_MAX;	//	距离无限远
		obj = nullptr;		//	其余属性都设为空
		m = nullptr;
		coords = vec3(0.f);
		normal = vec3(0.f);
		emit = vec3(0.f);
	}
};
