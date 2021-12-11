#pragma once
#include "Function.hpp"
#include "OBJ_LOADER.hpp"
#include "Object.hpp"
#include "Material.hpp"

//	三角形类
class Triangle : public Object {
public:
	vec3 v0, v1, v2;	//三个顶点
	vec3 e1, e2;		//两条边
	vec3 normal;		//法线
	float area;			//面积
	Material* m;		//材质

	Triangle(const vec3& _v0, const vec3& _v1, const vec3& _v2, Material* _m = nullptr)
		:v0(_v0), v1(_v1), v2(_v2), m(_m) {
		e1 = v1 - v0;
		e2 = v2 - v0;
		normal = cross(e1, e2).normalized();
		area = cross(e1, e2).norm() * 0.5f;
	}

	bool isIntersect(const Ray& ray) {
		return getIntersection(ray).happened;
	}

	Intersection getIntersection(const Ray& ray)  {
		// ray = O + tD
		// (1 - u - v) * v0 + u * v1 + v * v2 = O + tD ,u >= 0,v >= 0
		//  t                       S2 * E2     S = O - v0    |  S1 = D cross E2
		//  u   =   1/(S1 * E1)  *  S1 * S      E1 = v1 - v2  |  S2 = S cross E1
		//  v                       S2 * D      E2 = v2 - v0  |
		vec3 o = ray.Ori;
		vec3 d = ray.Dir;
		Intersection inter;

		if (normal.dot(d) > 0) {
			if (m->getType() == DIFFUSE || m->isMetal) {
				//	光线方向与法线方向一致的时候不可能与三角形正面相交
				return inter;
			}
			//vec3 u0 = v0;
			//vec3 u1 = v2;
			//vec3 u2 = v1;
			//vec3 e1 = u1 - u0;
			//vec3 e2 = u2 - u0;
			//float u, v, t;
			//vec3 S1 = d.cross(e2);
			//float denom = S1.dot(e1);
			//if (abs(denom) < EPS) {
			//	return inter;
			//}
			//float denomInv = 1.f / denom;
			//vec3 S = o - u0;
			//u = S1.dot(S) * denomInv;
			//if (u < 0 || u > 1) {
			//	return inter;
			//}
			//vec3 S2 = S.cross(e1);
			//v = S2.dot(d) * denomInv;
			//if (v < 0 || v + u > 1) {
			//	return inter;
			//}
			//t = S2.dot(e2) * denomInv;
			//if (t < 0) {
			//	return inter;
			//}
			//inter.coords = ray(t);
			//inter.distance = t;
			//inter.m = m;
			//inter.normal = normal;
			//inter.obj = this;
			//inter.happened = true;
			//inter.emit = m->getEmission();
			//return inter;
		}

		float u, v, t;

		vec3 S1 = d.cross(e2);
		float denom = S1.dot(e1);
		if (abs(denom) < EPS) {
			return inter;
		}

		float denomInv = 1.f / denom;
		vec3 S = o - v0;
		u = S1.dot(S) * denomInv;
		if (u < 0 || u > 1) {
			return inter;
		}

		vec3 S2 = S.cross(e1);
		v = S2.dot(d) * denomInv;
		if (v < 0 || v + u > 1) {
			return inter;
		}

		t = S2.dot(e2) * denomInv;
		if (t < 0) {
			return inter;
		}

		inter.coords = ray(t);
		inter.distance = t;
		inter.m = m;
		inter.normal = normal;
		inter.obj = this;
		inter.happened = true;
		inter.emit = m->getEmission();
		return inter;
	}

	Bounds getBounds()const {
		return Union(Bounds(v0, v1), v2);
	}
	// 对三角形光源采样
	float Sample(Intersection& inter) const{
		float x = mysqrt(getrandom());
		float y = getrandom();

		float pdf = 1.f / area;

		inter.coords = v0 * (1 - x) + v1 * (x * (1 - y)) + v2 * x * y;
		inter.normal = normal;
		return pdf;
	}

	float getArea()const {
		return area;
	}

	bool hasEmit()const {
		return m->hasEmission();
	}
};