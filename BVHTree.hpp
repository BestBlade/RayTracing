#pragma once
#include "Bounds.hpp"
#include "Object.hpp"
#include "Intersection.hpp"
#include <cassert>
//	BVH节点
struct BVHNode {
	float area;		//	节点包含物体面积
	BVHNode* left;	//	节点左孩子
	BVHNode* right;	//	节点右孩子
	Object* obj;	//	节点包含物体类型
	Bounds bounds;	//	节点包含物体包围盒
	//	默认构造
	BVHNode() {
		bounds = Bounds();
		left = nullptr;
		right = nullptr;
		obj = nullptr;
	}
};

class BVHAccel {
private:
	//	不知道原代码写来干啥用的，我用来统计节点里包含多少三角形
	const int maxPrimsInNode;
	//	BVH树根节点
	BVHNode* root;
	//	图元存储
	std::vector<Object*> primitives;
	//	将存好图元的vector用来构建BVH树
	BVHNode* Build(std::vector<Object*> objects) {
		BVHNode* node = new BVHNode();
		//	叶节点
		if (objects.size() == 1) {
			node->area = objects[0]->getArea();
			node->bounds = objects[0]->getBounds();
			node->obj = objects[0];
			node->left = nullptr;
			node->right = nullptr;
			return node;
		}
		//	叶节点
		else if(objects.size() == 2) {
			node->left = Build(std::vector<Object*> {objects[0]});
			node->right = Build(std::vector<Object*> {objects[1]});
			node->area = node->left->area + node->right->area;
			node->bounds = Union(node->left->bounds, node->right->bounds);
			return node;
		}
		//	非叶节点
		else {			
			//	计算当前节点包围盒
			Bounds bounds;
			for (int i = 0; i < objects.size(); ++i) {
				bounds = Union(bounds, objects[i]->getBounds());
			}
			//	计算包围盒的XYZ方向最大值，然后按照最大方向排序，瓜分
			int dim = bounds.MaxElemLoc();

			if (dim == 0) {
				//	自定义排序，如果dim == 0，说明当前节点包围盒x方向最长
				//	沿着x方向排序
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
			//	将排好序的obj分为前后两部分
			auto begin = objects.begin();
			auto end = objects.end();
			auto mid = begin + objects.size() / 2;
			
			std::vector<Object*> left = std::vector<Object*>(begin, mid);
			std::vector<Object*> right = std::vector<Object*>(mid, end);

			assert(left.size() + right.size() == objects.size());
			//	递归创建左右子树
			node->left = Build(left);
			node->right = Build(right);
			node->bounds = bounds;
			node->area = node->left->area + node->right->area;
		}
		return node;
	}
public:
	enum class SplitMethod { NAIVE };	//	创建BVH树的模式
	const SplitMethod splitMethod;

	//	对传进来的object数组p要用move构造
	BVHAccel(std::vector<Object*> p, int maxPrimsInNode = 255
		, SplitMethod s = SplitMethod::NAIVE) 
		: maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(s) , primitives(std::move(p)) {
		time_t tStart, tEnd;
		time(&tStart);

		if (primitives.empty()) {
			return;
		}

		root = Build(primitives);	//	创建BVH树，其余的句子时计时的

		time(&tEnd);
		float diff = difftime(tEnd, tStart);
		int hour = diff / 3600;
		int min = diff / 60 - hour * 60;
		int sec = diff - hour * 3600 - min * 60;

		std::cout << "\r BVH Tree Generation Complete !\nTime cost : "
			<< hour << " hours " << min << " min " << sec << " sec \n\n";
	}
	//	判断光线是否与BVH树相交
	bool isIntersect(const Ray& ray)const {
		//	返回求交的结果
		return getIntersection(ray).happened;
	}
	//	光线与BVH树求交，返回交点inter
	Intersection getIntersection(const Ray& ray)const {
		Intersection inter;
		if (root == nullptr) {
			return inter;
		}
		return getIntersection(root, ray);
	} 
	//	递归，光线与节点node求交
	Intersection getIntersection(BVHNode* node, const Ray& ray)const {
		Intersection inter;

		auto [x, y, z] = ray.Dir;	//	C++17标准，结构化绑定
		std::array<bool, 3> dirIsNeg = { x > 0, y > 0, z > 0 };	//	计算光线是否反向	
		//	判断光线是否与节点包围盒相交，不相交直接返回inter
		if (!node->bounds.isIntersect(ray, dirIsNeg)) {
			return inter;
		}
		//	与叶结点求交
		if (!node->left && !node->right) {
			inter = node->obj->getIntersection(ray);
			return inter;
		}
		//	计算与左右子树的交点，返回最近的交点
		Intersection hit1 = getIntersection(node->left, ray);
		Intersection hit2 = getIntersection(node->right, ray);
		return hit1.distance < hit2.distance ? hit1 : hit2;
	}

	// 对物体采样，只有light用到了
	float Sample(Intersection& inter) {
		float p = mysqrt(getrandom()) * root->area;
		float pdf = getSample(root, p, inter);
		pdf /= root->area;
		return pdf;
	}
	//	递归对物体采样，只有light用到了
	float getSample(BVHNode* node, float p, Intersection& inter) {
		//	在叶结点采样
		if (node->left == nullptr || node->right == nullptr) {
			float pdf = node->obj->Sample(inter);
			pdf *= node->area;
			return pdf;
		}
		//	如果概率小于左边面积在左子树采样
		if (p < node->left->area) {
			return getSample(node->left, p, inter);
		}
		//	否则以p-左子树面积的概率在右子树采样
		else {
			return getSample(node->right, p - node->left->area, inter);
		}
	}
};