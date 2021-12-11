#pragma once
#include "Bounds.hpp"
#include "Ray.hpp"
#include "Intersection.hpp"

class Object {
public:
	Object() {};
	virtual ~Object() {};
	virtual bool isIntersect(const Ray& ray) = 0;				//	�ж��Ƿ������Ray�ཻ
	virtual Intersection getIntersection(const Ray& ray) = 0;	//	�󽻵�
	virtual Bounds getBounds()const = 0;						//	��ȡ��Χ��
	virtual float getArea()const = 0;							//	��ȡ���
	virtual float Sample(Intersection& p)const = 0;				//	�����������ֻ��light�õ���
	virtual bool hasEmit()const = 0;							//	�ж��Ƿ񷢹�
};