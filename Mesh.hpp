#pragma once
#include "Function.hpp"
#include "OBJ_LOADER.hpp"
#include "Material.hpp"
#include "Triangle.hpp"
#include "BVHTree.hpp"
#include "Object.hpp"
//	��������
class Mesh : public Object {
public:
	unsigned int numTriangles;	//	����������
	float area;					//	������������������
	BVHAccel* bvh;				//	���񹹽���BVH�����ڵ�
	Material* m;				//	��ǰ����
	Bounds boundingBox;			//	��Χ��

	std::vector<Triangle> triangles;	//	�洢�����ε�����

	Mesh(const std::string& filename, Material* _m = new Material()) {
		objl::Loader loader;
		loader.LoadFile(filename);	//	ʹ��OBJ_LOADER����ģ�ͣ�һ��.obj�ļ���һ��Mesh
		area = 0;
		m = _m;
		numTriangles = 0;

		assert(loader.LoadedMeshes.size() == 1);
		auto mesh = loader.LoadedMeshes[0];

		vec3 pMin(INT_MAX);
		vec3 pMax(INT_MIN);

		//	�������ж���
		for (int i = 0; i < mesh.Vertices.size(); i += 3) {
			vec3 face[3];

			for (int j = 0; j < 3; ++j) {
				vec3 tmp(
					mesh.Vertices[i + j].Position.X,
					mesh.Vertices[i + j].Position.Y,
					mesh.Vertices[i + j].Position.Z
				);

				face[j] = tmp;
				
				//	�����Χ�еĶ���
				pMin.x = std::min(pMin.x, tmp.x);
				pMin.y = std::min(pMin.y, tmp.y);
				pMin.z = std::min(pMin.z, tmp.z);

				pMax.x = std::max(pMax.x, tmp.x);
				pMax.y = std::max(pMax.y, tmp.y);
				pMax.z = std::max(pMax.z, tmp.z);
			}
			//	��face�д�������������triangles�У�ԭ�ع���
			triangles.emplace_back(face[0], face[1], face[2], _m);
			++numTriangles;	//	����+1
		}
		//	����Mesh��Χ��
		boundingBox = Bounds(pMin, pMax);

		std::vector<Object*> ptrs;
		//	�������δ���Object*�����У����������
		for (auto &tri : triangles) {
			ptrs.emplace_back(&tri);
			area += tri.area;
		}
		//	����Object*���鹹����ǰMesh��BVH��
		bvh = new BVHAccel(ptrs);
	}
	//	�ж��Ƿ��ཻ
	bool isIntersect(const Ray& ray)  {
		return getIntersection(ray).happened;
	}
	//	��ȡ�ཻ��
	Intersection getIntersection(const Ray& ray) {
		Intersection inter;
		if (bvh) {
			inter = bvh->getIntersection(ray);
		}
		return inter;
	}
	//	��ȡ��Χ��
	Bounds getBounds()const {
		return boundingBox;
	}
	//	��ȡ���
	float getArea()const {
		return area;
	}
	//	��Mesh������ֻ��light�õ���
	float Sample(Intersection& inter) const{
		float pdf = bvh->Sample(inter);
		inter.emit = m->getEmission();
		return pdf;
	}
	//	�ж����Mesh�Ƿ�ᷢ��
	bool hasEmit()const {
		return m->hasEmission();
	}
};