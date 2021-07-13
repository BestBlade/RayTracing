#pragma once
#include "Function.hpp"
#include "OBJ_LOADER.hpp"
#include "BVHTree.hpp"
#include "Intersection.hpp"
#include "Ray.hpp"
#include "Material.hpp"
#include "Vector.hpp"

class Triangle :public Object {
public:
	vec3 v0, v1, v2;	//三个顶点
	vec3 e1, e2;		//两个边 e1 = v1-v0,e2 = v2-v0
	vec3 t0, t1, t2;	//顶点贴图
	vec3 normal;		//法线
	float area;			//面积
	Material* m;		//材质	

	Triangle(vec3 v0_, vec3 v1_, vec3 v2_, Material* m_ = nullptr)
		:v0(v0_), v1(v1_), v2(v2_), m(m_) {
		e1 = v1 - v0;
		e2 = v2 - v0;
		normal = normalize(cross(e1, e2));
		area = norm(cross(e1, e2)) * 0.5f;
	}

	bool intersect(const Ray& ray, float& tnear, unsigned int& index) const override {
		return false;
	}

	//判断光线是否与三角面相交
	Intersection getIntersection(Ray ray) override {
	// ray = O + tD
	// (1 - u - v) * v0 + u * v1 + v * v2 = O + tD ,u >= 0,v >= 0
	//  t                       S2 * E2     S = O - v0    |  S1 = D cross E2
	//  u   =   1/(S1 * E1)  *  S1 * S      E1 = v1 - v2  |  S2 = S cross E1
	//  v                       S2 * D      E2 = v2 - v0  |
		Intersection inter;
		if (dot(ray.dir, normal) > 0) {
			//路径和法线是钝角，绝对不相交
			return inter;
		}
		float u, v, t_tmp;

		vec3 S1 = cross(ray.dir, e2);
		float det = dot(S1, e1);
		if (fabs(det) < EPSILON) {
			return inter;
		}

		float det_inv = 1.0f / det;
		vec3 S = ray.ori - v0;

		u = dot(S, S1) * det_inv;
		if (u < 0 || u > 1) {
			return inter;
		}

		vec3 S2 = cross(S, e1);
		v = dot(S2, ray.dir) * det_inv;
		if (v < 0 || u + v > 1) {
			return inter;
		}

		t_tmp = dot(S2, e2) * det_inv;
		if (t_tmp < 0) {
			return inter;
		}

		inter.coords = ray(t_tmp);
		inter.distance = t_tmp;
		inter.m = m;
		inter.normal = normal;
		inter.obj = this;
		inter.happened = true;
		inter.emit = m->getEmission();
		return inter;
	}
	//	三角面用不着用不着
	void getSurfaceProperties(const vec3& P, const vec3& I, const unsigned int& index, const vec2& uv, vec3& N, vec2& st) const override {
		N = normal;
	}

	//	求三角形的包围盒
	Bounds3 getBounds() override {
		return Union(Bounds3(v0, v1), v2);
	}
	//	对三角形采样
	void Sample(Intersection& pos, float& pdf) {
		//	采样并回传坐标和pdf
		float x = sqrt(get_random_float());
		float y = get_random_float();
		//	pdf为面积的倒数
		pdf = 1.0 / area;
		//	合计为1，还在三角形内
		pos.coords = v0 * (1.0f - x) + v1 * float(x * (1.0 - y)) + v2 * x * y;
		//	接触点的法线和三角形方向一致
		pos.normal = this->normal;
	}

	float getArea() {
		return area;
	}
	//是否发光
	bool hasEmit() {
		//得看材质是否能发光
		return m->hasEmission();
	}
};