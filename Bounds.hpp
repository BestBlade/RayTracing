#pragma once
#include "Vector.hpp"
#include "Ray.hpp"
#include <array>
//	��Χ����
class Bounds {
public:
	vec3 pMin, pMax;
	//	���캯��
	Bounds() {
		pMax = vec3(INT_MIN);
		pMin = vec3(INT_MAX);
	}
	Bounds(const vec3& p) :pMin(p), pMax(p) {}
	Bounds(const vec3& p, const vec3& q) {
		pMin = vec3(
			std::min(p.x, q.x),
			std::min(p.y, q.y),
			std::min(p.z, q.z)
		);
		pMax = vec3(
			std::max(p.x, q.x),
			std::max(p.y, q.y),
			std::max(p.z, q.z)
		);
	}

	//	��Խ���
	vec3 Diag() const {
		return pMax - pMin;
	};
	//	�����Ԫ��λ�ã����ڴ���BVH��ʱ�ָ��Χ��
	int MaxElemLoc() const {
		vec3 diag = this->Diag();
		if (diag.x > diag.y && diag.x > diag.z) {
			return 0;
		}
		else if (diag.y > diag.z) {
			return 1;
		}
		else {
			return 2;
		}
	};
	//	���Χ�б����
	float SurfaceArea()const {
		vec3 diag = this->Diag();
		return 2 * (
			diag.x * diag.y +
			diag.y * diag.z +
			diag.z * diag.x);
	};
	//	���Χ������
	vec3 CenterPos()const {
		return pMin * 0.5f + pMax * 0.5f;
	};
	//	��Χ����
	Bounds Intersect(const Bounds& b)const {
		vec3 ppMin(
			std::max(pMin.x, b.pMin.x),
			std::max(pMin.y, b.pMin.y),
			std::max(pMin.z, b.pMin.z)
		);
		vec3 ppMax(
			std::min(pMax.x, b.pMax.x),
			std::min(pMax.y, b.pMax.y),
			std::min(pMax.z, b.pMax.z)
		);
		return Bounds(ppMin, ppMax);
	};
	//	����ڰ�Χ�е�ƫ��
	vec3 Offset(const vec3& p)const {
		vec3 t = p - pMin;
		if (pMax.x > pMin.x) {
			t.x /= (pMax.x - pMin.x);
		}
		if (pMax.y > pMin.y) {
			t.y /= (pMax.y - pMin.y);
		}
		if (pMax.z > pMin.z) {
			t.z /= (pMax.z - pMin.z);
		}
		return t;
	};
	//	�жϹ����Ƿ����Χ���ཻ
	bool isIntersect(const Ray& ray, std::array<bool, 3> dirIsNeg = {1,1,1})const {
		vec3 posmin = pMin - ray.Ori;
		vec3 posmax = pMax - ray.Ori;

		float xmax = posmax.x * ray.Dirinv.x;
		float xmin = posmin.x * ray.Dirinv.x;
		float ymax = posmax.y * ray.Dirinv.y;
		float ymin = posmin.y * ray.Dirinv.y;
		float zmax = posmax.z * ray.Dirinv.z;
		float zmin = posmin.z * ray.Dirinv.z;

		if (!dirIsNeg[0]) {
			std::swap(xmax, xmin);
		}
		if (!dirIsNeg[1]) {
			std::swap(ymax, ymin);
		}
		if (!dirIsNeg[2]) {
			std::swap(zmax, zmin);
		}

		float tEnter = std::max({ xmin,ymin,zmin });
		float tExit = std::min({ xmax,ymax,zmax });

		return tExit >= tEnter && tExit >= 0;
	};
	//	ȡ��Χ�����/��С�㣬û�õ�
	inline const vec3 operator[](int i)const {
		if (i) {
			return pMax;
		}
		return pMin;
	}
};

//	�жϰ�Χ���Ƿ��ཻ
inline bool Overlaps(const Bounds& b1, const Bounds& b2) {
	bool x = (b1.pMin.x <= b2.pMax.x) && (b1.pMax.x >= b2.pMin.x);
	bool y = (b1.pMin.y <= b2.pMax.y) && (b1.pMax.y >= b2.pMin.y);
	bool z = (b1.pMin.z <= b2.pMax.z) && (b1.pMax.z >= b2.pMin.z);
	return (x && y && z);
}
//	�жϵ��Ƿ��ڰ�Χ���ڲ�
inline bool isInside(const Bounds& b1, const vec3& p) {
	return
		(b1.pMax.x >= p.x && b1.pMin.x <= p.x) &&
		(b1.pMax.y >= p.y && b1.pMin.y <= p.y) &&
		(b1.pMax.z >= p.z && b1.pMin.z <= p.z);
}
//	��Χ�����Χ�кϲ�
Bounds Union(const Bounds& p, const Bounds& q) {
	vec3 ppMin(
		std::min(p.pMin.x, q.pMin.x),
		std::min(p.pMin.y, q.pMin.y),
		std::min(p.pMin.z, q.pMin.z)
	);
	vec3 ppMax(
		std::max(p.pMax.x, q.pMax.x),
		std::max(p.pMax.y, q.pMax.y),
		std::max(p.pMax.z, q.pMax.z)
	);
	return Bounds(ppMin, ppMax);
}
//	��Χ����vec�ϲ�
Bounds Union(const Bounds& p, const vec3& q) {
	vec3 ppMin(
		std::min(p.pMin.x, q.x),
		std::min(p.pMin.y, q.y),
		std::min(p.pMin.z, q.z)
	);
	vec3 ppMax(
		std::max(p.pMax.x, q.x),
		std::max(p.pMax.y, q.y),
		std::max(p.pMax.z, q.z)
	);
	return Bounds(ppMin, ppMax);
}