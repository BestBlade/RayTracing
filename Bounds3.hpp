#pragma once
#include "Function.hpp"
#include "Vector.hpp"
#include "Ray.hpp"
//包围盒类
class Bounds3 {
public:
	//左上顶点和右下顶点
	vec3 pMin, pMax;

	Bounds3() {
		pMin = vec3(INT_MAX);
		pMax = vec3(INT_MIN);
	}

	Bounds3(const vec3& p) :pMin(p), pMax(p) {}

	Bounds3(const vec3& p, const vec3& q) {
		pMin = vec3(min(p.x, q.x), min(p.y, q.y), min(p.z, q.z));
		pMax = vec3(max(p.x, q.x), max(p.y, q.y), max(p.z, q.z));
	}
	//对角线
	vec3 Diag() const {
		return pMax - pMin;
	}
	//最大元素位置
	int MaxElemLoc() const {
		vec3 diag = Diag();
		if (diag.x > diag.y && diag.x > diag.z) {
			return 0;
		}
		else if (diag.y > diag.z) {
			return 1;
		}
		else {
			return 2;
		}
	}
	//总面积
	float SurfaceArea() const {
		vec3 diag = Diag();
		return 2 * (diag.x * diag.y + diag.x * diag.z + diag.y * diag.z);
	}
	//包围盒的中心
	vec3 CenterPos() const {
		return pMin * 0.5f + pMax * 0.5f;
	}
	//判断是否重叠
	bool Overlaps(const Bounds3& b1, const Bounds3& b2) {
		//b1的右上角要大于等于b2的左下角，并且b2的右上角要大于等于b1的左下角
		bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
		bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
		bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
		return (x && y && z);
	}
	//包围盒的交集
	Bounds3 Intersect(const Bounds3& b) {
		vec3 pmin_(fmax(pMin.x, b.pMin.x), fmax(pMin.y, b.pMin.y), fmax(pMin.z, b.pMin.z));
		vec3 pmax_(fmin(pMax.x, b.pMax.x), fmin(pMax.y, b.pMax.y), fmin(pMax.z, b.pMax.z));
		return Bounds3(pmin_, pmax_);
	}

	vec3 Offset(const vec3& p) const {
		vec3 res = p - pMin;

		if (pMin.x < pMax.x) {
			res.x /= pMax.x - pMin.x;
		}
		if (pMin.y < pMax.y) {
			res.y /= pMax.y - pMin.y;
		}
		if (pMin.z < pMax.z) {
			res.z /= pMax.z - pMin.z;
		}
		return res;
	}
	//判断点p是否在包围盒内
	bool Inside(const vec3& p, const Bounds3& b) {
		return ((p.x <= b.pMax.x) && (p.x >= b.pMin.x)
			&& (p.y <= b.pMax.y) && (p.y >= b.pMin.y)
			&& (p.z <= b.pMax.z) && (p.z >= b.pMin.z));
	}

	inline const vec3 operator[](int i) const {
		return i == 0 ? pMin : pMax;
	}
	//判断光线是否与包围盒相交
	inline bool Intersect(const Ray& ray, const vec3& invDir, const int dirIsNeg[3])const {
		auto t0 = (pMin - ray.ori) * invDir;
		auto t1 = (pMax - ray.ori) * invDir;

		auto v0 = vec3(min(t0.x, t1.x), min(t0.y, t1.y), min(t0.z, t1.z));
		auto v1 = vec3(max(t0.x, t1.x), max(t0.y, t1.y), max(t0.z, t1.z));

		float tenter = max(v0.x, max(v0.y, v0.z));
		float texit = min(v1.x, min(v1.y, v1.z));
		return tenter <= texit && texit > 0;
	}
};

inline Bounds3 Union(const Bounds3& b1, const Bounds3& b2) {
	Bounds3 res;
	res.pMin = vec3(min(b1.pMin.x, b2.pMin.x), min(b1.pMin.y, b2.pMin.y), min(b1.pMin.z, b2.pMin.z));
	res.pMax = vec3(max(b1.pMax.x, b2.pMax.x), max(b1.pMax.y, b2.pMax.y), max(b1.pMax.z, b2.pMax.z));
	return res;
}

inline Bounds3 Union(const Bounds3& b, const vec3& p) {
	Bounds3 res;
	res.pMin = vec3(min(b.pMin.x, p.x), min(b.pMin.y, p.y), min(b.pMin.z, p.z));
	res.pMax = vec3(max(b.pMax.x, p.x), max(b.pMax.y, p.y), max(b.pMax.z, p.z));
	return res;
}
