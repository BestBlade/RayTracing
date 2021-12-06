#pragma once
#include "Function.hpp"
#include "OBJ_LOADER.hpp"
#include "Material.hpp"
#include "Triangle.hpp"
#include "BVHTree.hpp"
#include "Object.hpp"
//	网格面类
class Mesh : public Object {
public:
	unsigned int numTriangles;	//	三角形数量
	float area;					//	网格包含三角形总面积
	BVHAccel* bvh;				//	网格构建的BVH树根节点
	Material* m;				//	当前材质
	Bounds boundingBox;			//	包围盒

	std::vector<Triangle> triangles;	//	存储三角形的数组

	Mesh(const std::string& filename, Material* _m = new Material()) {
		objl::Loader loader;
		loader.LoadFile(filename);	//	使用OBJ_LOADER加载模型，一个.obj文件是一个Mesh
		area = 0;
		m = _m;
		numTriangles = 0;

		assert(loader.LoadedMeshes.size() == 1);
		auto mesh = loader.LoadedMeshes[0];

		vec3 pMin(INT_MAX);
		vec3 pMax(INT_MIN);

		//	遍历所有顶点
		for (int i = 0; i < mesh.Vertices.size(); i += 3) {
			vec3 face[3];

			for (int j = 0; j < 3; ++j) {
				vec3 tmp(
					mesh.Vertices[i + j].Position.X,
					mesh.Vertices[i + j].Position.Y,
					mesh.Vertices[i + j].Position.Z
				);

				face[j] = tmp;
				
				//	计算包围盒的顶点
				pMin.x = std::min(pMin.x, tmp.x);
				pMin.y = std::min(pMin.y, tmp.y);
				pMin.z = std::min(pMin.z, tmp.z);

				pMax.x = std::max(pMax.x, tmp.x);
				pMax.y = std::max(pMax.y, tmp.y);
				pMax.z = std::max(pMax.z, tmp.z);
			}
			//	将face中存的三个顶点存入triangles中，原地构造
			triangles.emplace_back(face[0], face[1], face[2], _m);
			++numTriangles;	//	计数+1
		}
		//	构建Mesh包围盒
		boundingBox = Bounds(pMin, pMax);

		std::vector<Object*> ptrs;
		//	将三角形存入Object*数组中，计算总面积
		for (auto &tri : triangles) {
			ptrs.emplace_back(&tri);
			area += tri.area;
		}
		//	利用Object*数组构建当前Mesh的BVH树
		bvh = new BVHAccel(ptrs);
	}
	//	判断是否相交
	bool isIntersect(const Ray& ray)  {
		return getIntersection(ray).happened;
	}
	//	获取相交点
	Intersection getIntersection(const Ray& ray) {
		Intersection inter;
		if (bvh) {
			inter = bvh->getIntersection(ray);
		}
		return inter;
	}
	//	或取包围盒
	Bounds getBounds()const {
		return boundingBox;
	}
	//	获取面积
	float getArea()const {
		return area;
	}
	//	对Mesh采样，只有light用到了
	float Sample(Intersection& inter) const{
		float pdf = bvh->Sample(inter);
		inter.emit = m->getEmission();
		return pdf;
	}
	//	判断这个Mesh是否会发光
	bool hasEmit()const {
		return m->hasEmission();
	}
};
