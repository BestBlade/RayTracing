#pragma once
#include "Function.hpp"
#include "OBJ_LOADER.hpp"
#include "BVHTree.hpp"
#include "Bounds3.hpp"
#include "Intersection.hpp"
#include "Ray.hpp"
#include "Material.hpp"
#include "Triangle.hpp"

class Mesh :public Object {
public:
	Bounds3 bounding_box;
	unsigned int numTriangles;

	std::unique_ptr<vec3[]> vertices;
	std::unique_ptr<unsigned int[]> vertexIndex;
	std::unique_ptr<vec2[]> stCoordinates;

	std::vector<Triangle> triangles;

	BVHAccel* bvh;
	float area;

	Material* m;

	Mesh(const std::string& filename, Material* m_ = new Material()) {
		objl::Loader loader;
		loader.LoadFile(filename);
		area = 0;
		m = m_;
		assert(loader.LoadedMeshes.size() == 1);
		auto mesh = loader.LoadedMeshes[0];
		//最小顶点和最大顶点
		vec3 minVert(INT_MAX);
		vec3 maxVert(INT_MIN);

		for (int i = 0; i < mesh.Vertices.size(); i += 3) {
			vec3 face[3];

			for (int j = 0; j < 3; ++j) {
				vec3 tmp(
					mesh.Vertices[i + j].Position.X,
					mesh.Vertices[i + j].Position.Y,
					mesh.Vertices[i + j].Position.Z);

				face[j] = tmp;

				minVert = vec3(
					min(tmp.x, minVert.x),
					min(tmp.y, minVert.y),
					min(tmp.z, minVert.z));
				maxVert = vec3(
					max(tmp.x, maxVert.x),
					max(tmp.y, maxVert.y),
					max(tmp.z, maxVert.z));
			}

			triangles.emplace_back(face[0], face[1], face[2], m_);
		}
		//当前Mesh的包围盒
		bounding_box = Bounds3(minVert, maxVert);

		//存储每个三角形
		//std::vector<Object*> ptrs(triangles.size());
		//for (int i = 0; i < triangles.size(); ++i) {
		//	ptrs[i] = &triangles[i];
		//	area += triangles[i].area;
		//}
		std::vector<Object*> ptrs;
		for (auto& tri : triangles) {
			ptrs.emplace_back(&tri);
			//Mesh的面积为所有三角形面积的和
			area += tri.area;
		}
		bvh = new BVHAccel(ptrs);
	}

	//	遍历Mesh中的每一个三角形，判断是否与光线相交
	bool intersect(const Ray& ray, float& tnear, unsigned int& index) const {
		bool isIntersect = false;
		//	numTriangles从何而来
		for (int i = 0; i < numTriangles; ++i) {
			const vec3& v0 = vertices[vertexIndex[i * 3]];
			const vec3& v1 = vertices[vertexIndex[i * 3 + 1]];
			const vec3& v2 = vertices[vertexIndex[i * 3 + 2]];

			float t, u, v;
			if (rayTriangleIntersect(v0, v1, v2, ray.ori, ray.dir, t, u, v) && t < tnear) {
				tnear = t;
				index = i;
				isIntersect = 1;
			}
		}
		return isIntersect;
	}

	Bounds3 getBounds() {
		return bounding_box;
	}

	float getArea() {
		return area;
	}

	//	计算相交位置
	Intersection getIntersection(Ray ray) {
		Intersection inter;
		if (bvh) {
			inter = bvh->Intersect(ray);
		}
		return inter;
	}

	//	在相交点采样
	void Sample(Intersection& pos, float& pdf) {
		bvh->Sample(pos, pdf);
		pos.emit = m->getEmission();
	}

	bool hasEmit() {
		return m->hasEmission();
	}

	//	获得平面属性
	void getSurfaceProperties(const vec3& P, const vec3& I, const unsigned int& index, const vec2& uv, vec3& N, vec2& st) const {
		//	第index个三角形的顶点
		const vec3& v0 = vertices[vertexIndex[index * 3]];
		const vec3& v1 = vertices[vertexIndex[index * 3 + 1]];
		const vec3& v2 = vertices[vertexIndex[index * 3 + 2]];
		vec3 e0 = vec3(v1 - v0);
		vec3 e1 = vec3(v2 - v1);
		N = normalize(cross(e0, e1));

		const vec2& st0 = stCoordinates[vertexIndex[index * 3]];
		const vec2& st1 = stCoordinates[vertexIndex[index * 3 + 1]];
		const vec2& st2 = stCoordinates[vertexIndex[index * 3 + 2]];
		st = st0 * (1 - uv.x - uv.y) + st1 * uv.x + st2 * uv.y;
	}
};