#pragma once
#include "Object.hpp"
#include "Material.hpp"

//	球面类，未测试是否正确
class Sphere :public Object {
public:
	float radius, radius2;		//	定义半径r，半径平方r2
	float area;					//	球表面积
	Material* m;				//	球的材质
	vec3 center;				//	球中心位置

	Sphere(const vec3& c, float r, Material* m = new Material()) 
		:center(c), radius(r), radius2(r* r), m(m), area(4 * PI * radius2) {}

	bool isIntersect(const Ray& ray) {
		return getIntersection(ray).happened;
	}
	//	判断光线是否与球相交,相较于解方程方法有误差,但不会因为t过小导致奇怪的纹路
	Intersection getIntersection(const Ray& ray) {
		// O
		//	↘（t1）
		//	 ↑↘
		//	 ↑  ↘
		//	 C→	→(t2)		

		//	假设从O到C为S，则S与D夹角为锐角才有可能光线与球相交，并且O要在球外部
		//	计算S在D上的投影L，根据S^2-L^2可以计算出圆心C到光线的距离h^2,如果h^2 > r^2说明没有交点
		//	根据r^2-h^2可以求出割线长度的一半
		//	可以根据S在D上的投影l，通过+x和-x计算出两个接触点，-x为近的接触点
		vec3 o = ray.Ori;
		vec3 d = ray.Dir;
		Intersection inter;
		vec3 S = center - o;
		float S2 = S.dot(S);
		float l = S.dot(ray.Dir);

		if (l < 0.f && S2 > radius2) {
			return inter;
		}
		float h2 = S2 - l * l;
		if (h2 > radius2) {
			return inter;
		}
		float x = mysqrt(radius2 - h2);
		float t;
		if (S2 > radius2) {
			t = l - x;
		}
		else {
			if (m->getType() == DIFFUSE || m->isMetal) {
				return inter;
			}
			t = x + l;
		}
		if (t < EPS) {
			return inter;
		}

		inter.happened = true;
		inter.coords = ray(t);
		inter.distance = t;
		inter.emit = m->getEmission();
		inter.m = this->m;
		inter.normal = (inter.coords - center).normalized();
		inter.obj = this;
		return inter;
	}
	//	判断光线是否与球相交,解方程版本
	Intersection getIntersection1(const Ray& ray) {
		Intersection inter;
		vec3 S = ray.Ori - center;
		float a = 1.f;
		//float a = ray.Dir.dot(ray.Dir);
		float b = 2 * S.dot(ray.Dir);
		float c = S.dot(S) - radius2;
		float t0, t1;
		if (!solveQuadratic(a, b, c, t0, t1)) {
			return inter;
		}
		if (t0 < 0.f) { 
			std::swap(t0, t1);
		}
		if (t0 < EPS) {
			return inter;
		}

		inter.happened = true;
		inter.coords = ray(t0);
		inter.normal = (inter.coords - center).normalized();
		inter.m = m;
		inter.obj = this;
		inter.distance = t0;
		inter.emit = m->getEmission();
		return inter;
	}

	Bounds getBounds() const {
		vec3 pMax = center + radius * mysqrt(3);
		vec3 pMin = center - radius * mysqrt(3);
		return Bounds(pMin, pMax);
	}
	// 对球面采样
	float Sample(Intersection & inter) const{
		float pdf = 1. / area;
		// 球面均匀采样
		// theta在0-2pi上均匀采样
		// phi = arccos(1 - 2 * t),t在0-1上均匀采样
		float costheta = cos(getrandom() * 2 * PI);
		float sintheta = mysqrt(1.f - costheta * costheta);
		float cosphi = 1.f - 2 * getrandom();
		float sinphi = mysqrt(1.f - cosphi * cosphi);

		float x = radius * sinphi * costheta;
		float y = radius * sinphi * sintheta;
		float z = radius * cosphi;
		inter.coords = vec3(x, y, z) + center;
		inter.normal = (inter.coords - center).normalized();
		inter.emit = m->getEmission();
		return pdf;
	}
	float getArea()const {
		return area;
	}	
	bool hasEmit()const{
		return m->hasEmission();
	}
};