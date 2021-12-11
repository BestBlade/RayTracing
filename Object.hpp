#pragma once
#include "Bounds.hpp"
#include "Ray.hpp"
#include "Intersection.hpp"

class Object {
public:
	Object() {};
	virtual ~Object() {};
	virtual bool isIntersect(const Ray& ray) = 0;				//	判断是否与光线Ray相交
	virtual Intersection getIntersection(const Ray& ray) = 0;	//	求交点
	virtual Bounds getBounds()const = 0;						//	获取包围盒
	virtual float getArea()const = 0;							//	获取面积
	virtual float Sample(Intersection& p)const = 0;				//	对物体采样，只有light用到了
	virtual bool hasEmit()const = 0;							//	判断是否发光
};