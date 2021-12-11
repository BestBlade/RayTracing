#pragma once
#include "Vector.hpp"
class Ray {
public:
	float t;			//	O + tD
	float tMin, tMax;
	vec3 Ori;
	vec3 Dir;
	vec3 Dirinv;		//	(1/x, 1/y, 1/z)


	Ray(const vec3& ori, const vec3& dir,float tt = 0.f) 
		:Ori(ori), Dir(dir), t(tt), tMin(0.f), tMax(std::numeric_limits<float>::max()) {
		Dirinv.x = 1.f / Dir.x;
		Dirinv.y = 1.f / Dir.y;
		Dirinv.z = 1.f / Dir.z;
	}

	Ray(const Ray& r) :Ori(r.Ori), Dir(r.Dir), Dirinv(r.Dirinv), t(r.t), tMin(r.tMin), tMax(r.tMax) {}

	vec3 operator()(float tt) const {
		//	计算交点用:ray(t)
		return Ori + Dir * tt;
	}
};