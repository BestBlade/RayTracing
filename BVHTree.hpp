#pragma once
#include "Bounds.hpp"
#include "Object.hpp"
#include "Intersection.hpp"
#include <cassert>
//	BVH�ڵ�
struct BVHNode {
	float area;		//	�ڵ�����������
	BVHNode* left;	//	�ڵ�����
	BVHNode* right;	//	�ڵ��Һ���
	Object* obj;	//	�ڵ������������
	Bounds bounds;	//	�ڵ���������Χ��
	//	Ĭ�Ϲ���
	BVHNode() {
		bounds = Bounds();
		left = nullptr;
		right = nullptr;
		obj = nullptr;
	}
};

class BVHAccel {
private:
	//	��֪��ԭ����д����ɶ�õģ�������ͳ�ƽڵ����������������
	const int maxPrimsInNode;
	//	BVH�����ڵ�
	BVHNode* root;
	//	ͼԪ�洢
	std::vector<Object*> primitives;
	//	�����ͼԪ��vector��������BVH��
	BVHNode* Build(std::vector<Object*> objects) {
		BVHNode* node = new BVHNode();
		//	Ҷ�ڵ�
		if (objects.size() == 1) {
			node->area = objects[0]->getArea();
			node->bounds = objects[0]->getBounds();
			node->obj = objects[0];
			node->left = nullptr;
			node->right = nullptr;
			return node;
		}
		//	Ҷ�ڵ�
		else if(objects.size() == 2) {
			node->left = Build(std::vector<Object*> {objects[0]});
			node->right = Build(std::vector<Object*> {objects[1]});
			node->area = node->left->area + node->right->area;
			node->bounds = Union(node->left->bounds, node->right->bounds);
			return node;
		}
		//	��Ҷ�ڵ�
		else {			
			//	���㵱ǰ�ڵ��Χ��
			Bounds bounds;
			for (int i = 0; i < objects.size(); ++i) {
				bounds = Union(bounds, objects[i]->getBounds());
			}
			//	�����Χ�е�XYZ�������ֵ��Ȼ������������򣬹Ϸ�
			int dim = bounds.MaxElemLoc();

			if (dim == 0) {
				//	�Զ����������dim == 0��˵����ǰ�ڵ��Χ��x�����
				//	����x��������
				sort(objects.begin(), objects.end(), [&](Object* obj1, Object* obj2) {
					return obj1->getBounds().Diag().x < obj2->getBounds().Diag().x;
					});
			}
			else if (dim == 1) {
				sort(objects.begin(), objects.end(), [&](Object* obj1, Object* obj2) {
					return obj1->getBounds().Diag().y < obj2->getBounds().Diag().y;
					});
			}
			else {
				sort(objects.begin(), objects.end(), [&](Object* obj1, Object* obj2) {
					return obj1->getBounds().Diag().z < obj2->getBounds().Diag().z;
					});
			}
			//	���ź����obj��Ϊǰ��������
			auto begin = objects.begin();
			auto end = objects.end();
			auto mid = begin + objects.size() / 2;
			
			std::vector<Object*> left = std::vector<Object*>(begin, mid);
			std::vector<Object*> right = std::vector<Object*>(mid, end);

			assert(left.size() + right.size() == objects.size());
			//	�ݹ鴴����������
			node->left = Build(left);
			node->right = Build(right);
			node->bounds = bounds;
			node->area = node->left->area + node->right->area;
		}
		return node;
	}
public:
	enum class SplitMethod { NAIVE };	//	����BVH����ģʽ
	const SplitMethod splitMethod;

	//	�Դ�������object����pҪ��move����
	BVHAccel(std::vector<Object*> p, int maxPrimsInNode = 255
		, SplitMethod s = SplitMethod::NAIVE) 
		: maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(s) , primitives(std::move(p)) {
		time_t tStart, tEnd;
		time(&tStart);

		if (primitives.empty()) {
			return;
		}

		root = Build(primitives);	//	����BVH��������ľ���ʱ��ʱ��

		time(&tEnd);
		float diff = difftime(tEnd, tStart);
		int hour = diff / 3600;
		int min = diff / 60 - hour * 60;
		int sec = diff - hour * 3600 - min * 60;

		std::cout << "\r BVH Tree Generation Complete !\nTime cost : "
			<< hour << " hours " << min << " min " << sec << " sec \n\n";
	}
	//	�жϹ����Ƿ���BVH���ཻ
	bool isIntersect(const Ray& ray)const {
		//	�����󽻵Ľ��
		return getIntersection(ray).happened;
	}
	//	������BVH���󽻣����ؽ���inter
	Intersection getIntersection(const Ray& ray)const {
		Intersection inter;
		if (root == nullptr) {
			return inter;
		}
		return getIntersection(root, ray);
	} 
	//	�ݹ飬������ڵ�node��
	Intersection getIntersection(BVHNode* node, const Ray& ray)const {
		Intersection inter;

		auto [x, y, z] = ray.Dir;	//	C++17��׼���ṹ����
		std::array<bool, 3> dirIsNeg = { x > 0, y > 0, z > 0 };	//	��������Ƿ���	
		//	�жϹ����Ƿ���ڵ��Χ���ཻ�����ֱཻ�ӷ���inter
		if (!node->bounds.isIntersect(ray, dirIsNeg)) {
			return inter;
		}
		//	��Ҷ�����
		if (!node->left && !node->right) {
			inter = node->obj->getIntersection(ray);
			return inter;
		}
		//	���������������Ľ��㣬��������Ľ���
		Intersection hit1 = getIntersection(node->left, ray);
		Intersection hit2 = getIntersection(node->right, ray);
		return hit1.distance < hit2.distance ? hit1 : hit2;
	}

	// �����������ֻ��light�õ���
	float Sample(Intersection& inter) {
		float p = mysqrt(getrandom()) * root->area;
		float pdf = getSample(root, p, inter);
		pdf /= root->area;
		return pdf;
	}
	//	�ݹ�����������ֻ��light�õ���
	float getSample(BVHNode* node, float p, Intersection& inter) {
		//	��Ҷ������
		if (node->left == nullptr || node->right == nullptr) {
			float pdf = node->obj->Sample(inter);
			pdf *= node->area;
			return pdf;
		}
		//	�������С��������������������
		if (p < node->left->area) {
			return getSample(node->left, p, inter);
		}
		//	������p-����������ĸ���������������
		else {
			return getSample(node->right, p - node->left->area, inter);
		}
	}
};