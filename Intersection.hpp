#pragma once
#include "Vector.hpp"
#include "Material.hpp"
//	����Object�࣬����ͷ�ļ��������
class Object;
//	������
class Intersection {
public:
	bool happened;	//	�ཻ�Ƿ���
	float distance;	//	�ཻ����
	Object* obj;	//	�ཻ����������
	Material* m;	//	�ཻ�����

	vec3 coords;	//	�ཻ������
	vec3 normal;	//	�ཻ�㷨����
	vec3 emit;		//	�ཻ�㷢������
	//	Ĭ�Ϲ��캯��
	Intersection() {
		happened = false;	//	�ཻδ����
		distance = INT_MAX;	//	��������Զ
		obj = nullptr;		//	�������Զ���Ϊ��
		m = nullptr;
		coords = vec3(0.f);
		normal = vec3(0.f);
		emit = vec3(0.f);
	}
};