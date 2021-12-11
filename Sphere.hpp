#pragma once
#include "Object.hpp"
#include "Material.hpp"

//	�����࣬δ�����Ƿ���ȷ
class Sphere :public Object {
public:
	float radius, radius2;		//	����뾶r���뾶ƽ��r2
	float area;					//	������
	Material* m;				//	��Ĳ���
	vec3 center;				//	������λ��

	Sphere(const vec3& c, float r, Material* m = new Material()) 
		:center(c), radius(r), radius2(r* r), m(m), area(4 * PI * radius2) {}

	bool isIntersect(const Ray& ray) {
		return getIntersection(ray).happened;
	}
	//	�жϹ����Ƿ������ཻ,����ڽⷽ�̷��������,��������Ϊt��С������ֵ���·
	Intersection getIntersection(const Ray& ray) {
		// O
		//	�K��t1��
		//	 ���K
		//	 ��  �K
		//	 C��	��(t2)		

		//	�����O��CΪS����S��D�н�Ϊ��ǲ��п��ܹ��������ཻ������OҪ�����ⲿ
		//	����S��D�ϵ�ͶӰL������S^2-L^2���Լ����Բ��C�����ߵľ���h^2,���h^2 > r^2˵��û�н���
		//	����r^2-h^2����������߳��ȵ�һ��
		//	���Ը���S��D�ϵ�ͶӰl��ͨ��+x��-x����������Ӵ��㣬-xΪ���ĽӴ���
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
	//	�жϹ����Ƿ������ཻ,�ⷽ�̰汾
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
	// ���������
	float Sample(Intersection & inter) const{
		float pdf = 1. / area;
		// ������Ȳ���
		// theta��0-2pi�Ͼ��Ȳ���
		// phi = arccos(1 - 2 * t),t��0-1�Ͼ��Ȳ���
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