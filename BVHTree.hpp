#pragma once
#include "Function.hpp"
#include "Vector.hpp"
#include "Object.hpp"
#include "Bounds3.hpp"

struct BVHNode {
	Bounds3 bounds;
	BVHNode* left;
	BVHNode* right;
	Object* obj;
	float area;

public:
	BVHNode() {
		bounds = Bounds3();
		left = nullptr;
		right = nullptr;
		obj = nullptr;
	}
};

class BVHAccel {
private:
	// BVHAccel Private Data
	const int maxPrimsInNode;
	std::vector<Object*> primitives;

	BVHNode* Build(std::vector<Object*> objects) {
		BVHNode* node = new BVHNode();
		Bounds3 bounds;
		//	求当前节点的包围盒
		for (int i = 0; i < objects.size(); ++i) {
			bounds = Union(bounds, objects[i]->getBounds());
		}
		//递归出口
		if (objects.size() == 1) {
			//	叶结点
			node->area = objects[0]->getArea();
			node->bounds = objects[0]->getBounds();
			node->obj = objects[0];
			node->left = nullptr;
			node->right = nullptr;
			return node;
		}
		else if (objects.size() == 2) {
			node->left = Build(std::vector<Object*> {objects[0]});
			node->right = Build(std::vector<Object*>{objects[1]});

			node->area = node->left->area + node->right->area;
			node->bounds = Union(node->left->bounds, node->right->bounds);
			return node;
		}
		else {
			//中心包围盒
			Bounds3 centerBounds;
			for (int i = 0; i < objects.size(); ++i) {
				centerBounds = Union(centerBounds, objects[i]->getBounds().CenterPos());
			}
			//计算中心包围盒的最大值，然后排序，瓜分
			int dim = centerBounds.MaxElemLoc();
			if (dim == 0) {
				std::sort(objects.begin(), objects.end(), [&](Object* f1, Object* f2) {
					return f1->getBounds().CenterPos().x < f2->getBounds().CenterPos().x;
					});
			}
			else if (dim == 1) {
				std::sort(objects.begin(), objects.end(), [&](Object* f1, Object* f2) {
					return f1->getBounds().CenterPos().y < f2->getBounds().CenterPos().y;
					});
			}
			else {
				std::sort(objects.begin(), objects.end(), [&](Object* f1, Object* f2) {
					return f1->getBounds().CenterPos().z < f2->getBounds().CenterPos().z;
					});
			}

			auto b = objects.begin();
			auto e = objects.end();
			auto mid = b + objects.size() / 2;

			auto le = std::vector<Object*>(b, mid);
			auto ri = std::vector<Object*>(mid, e);

			assert(objects.size() == le.size() + ri.size());

			node->left = Build(le);
			node->right = Build(ri);
			node->bounds = Union(node->left->bounds, node->right->bounds);
			node->area = node->left->area + node->right->area;
		}
		return node;
	}
public:
	enum class SplitMethod { NAIVE };
	const SplitMethod splitMethod;

	BVHNode* root;
	//	建树
	BVHAccel(std::vector<Object*> p, int maxPrimsInNode = 255, SplitMethod s = SplitMethod::NAIVE) :maxPrimsInNode(min(255, maxPrimsInNode)), splitMethod(s), primitives(std::move(p)) {
		time_t start, stop;
		time(&start);
		if (primitives.empty()) {
			return;
		}
		root = Build(primitives);

		time(&stop);
		double diff = difftime(stop, start);
		int hrs = (int)diff / 3600;
		int mins = ((int)diff / 60) - (hrs * 60);
		int secs = (int)diff - (hrs * 3600) - (mins * 60);

		std::cout << "\rBVH Generation complete: \nTime Taken:" << hrs << " hrs," << mins << " mins," << secs << " secs\n\n";
	}

	//	判断是否相交
	bool isIntersect(const Ray& ray) const {

	}

	//	树和光线求交点
	Intersection Intersect(const Ray& ray) const {
		Intersection inter;
		//	节点为空直接返回
		if (!root) {
			return inter;
		}
		//	否则和节点求交
		return getIntersection(root, ray);
	}

	//	光线和节点求交
	Intersection getIntersection(BVHNode* node, const Ray& ray) const {
		Intersection inter;

		float x = ray.dir.x;
		float y = ray.dir.y;
		float z = ray.dir.z;
		int dirIsNeg[3] = { x > 0, y > 0, z > 0 };

		//如果不和包围盒相交直接返回
		if (!node->bounds.Intersect(ray, ray.dir_inv, dirIsNeg)) {
			return inter;
		}
		//如果为叶子节点，计算光线与包围盒内物体是否相交
		if (!node->left && !node->right) {
			inter = node->obj->getIntersection(ray);
			return inter;
		}

		auto hit1 = getIntersection(node->left, ray);
		auto hit2 = getIntersection(node->right, ray);
		return hit1.distance < hit2.distance ? hit1 : hit2;
	}

	//	对节点进行采样
	void getSample(BVHNode* node, float p, Intersection& inter, float& pdf) {
		if (node->left == nullptr && node->right == nullptr) {
			node->obj->Sample(inter, pdf);
			pdf *= node->area;
			return;
		}
		if (p < node->left->area) {
			getSample(node->left, p, inter, pdf);
		}
		else {
			getSample(node->right, p, inter, pdf);
		}

	}

	//	对树进行采样
	void Sample(Intersection& inter, float& pdf) {
		//以概率 p*面积 进行采样
		float p = sqrt(get_random_float()) * root->area;
		getSample(root, p, inter, pdf);
		pdf /= root->area;
	}
};