#pragma once
#include <cmath>
#include <random>
#include <iostream>
#include <cassert>

const float EPS = 1e-6;
const float PI = 3.1415926535;
const float PI_INV = 1 / PI;
float mysqrt(float x);
//	������1/��x��ע��û�п���x<0����������Ǿ��Ȳ���
inline float mysqrtinv(float x) {
	//	https://www.cnblogs.com/signal/p/3818332.html
	float xhalf = 0.5f * x;
	int i = *(int*)&x;
	i = 0x5f37642f - (i >> 1);	// �����һ�����Ƹ�
	x = *(float*)&i;
	x = x * (1.5f - xhalf * x * x);	// ţ�ٵ�����
	return x;
}
//	�������x��ע��û�п���x<0�����
inline float mysqrt(float x) {
	//	https://blog.csdn.net/xtlisk/article/details/51249371
	//	��׼�����׼��,���Ǻ����󽻻������ֵ�bug
	//return std::sqrtf(x);
	//	������ֵƫС
	//return 1 / mysqrtinv(x);	//1/��x
	//	������ֵƫ��
	//return x * mysqrtinv(x);	//x*��x

	//	������ֵƫ��
	//	x86�޶�
	//float	result;
	//_asm
	//{
	//	mov eax, x
	//	sub eax, 0x3f800000
	//	sar eax, 1
	//	add eax, 0x3f800000
	//	mov result, eax
	//}
	//return result;

	//	������ֵƫС
	//	�������Ҳ���Խ���
	float y = sqrtf(x);

	float a = x;
	unsigned int i = *(unsigned int*)&x;
	i = (i + 0x3f76cf62) >> 1;
	x = *(float*)&i;
	x = (x + a / x) * 0.5;
	return (x + y) * 0.5f;
}
inline float pow5(float x) {
	return x * x * x * x * x;
}
//	�Ƕ�ת����
inline float radians(float degree) {
	return degree * PI / 180;
}
//	��ȡ0-1�������ʹ��static�Ż�����
static inline float getrandom() {
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_real_distribution<float> dist(0.f, 1.f); // distribution in range [1, 6]

	return dist(rng);
}

inline  bool solveQuadratic(const float& a, const float& b, const float& c, float& x0, float& x1) {
	float delta = b * b - 4 * a * c;
	if (delta < 0.f) {
		return false;
	}
	else if (delta < 0.000001f) {
		x0 = x1 = -0.5f * b / a;
	}
	else {
		float q = (b > 0) ?
			-0.5f * (b + mysqrt(delta)) :
			-0.5f * (b - mysqrt(delta));
		x0 = q / a;
		x1 = c / q;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
	}
	return true;
}
//	��Ⱦ���ȿ��ӻ�1
inline void Visualize(float progress)
{
	int barWidth = 100;

	std::cout << "[";
	int pos = barWidth * progress;
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) std::cout << "=";
		else if (i == pos) std::cout << ">";
		else std::cout << " ";
	}
	std::cout << "] " << int(progress * 100.0f) << " %\r";
	std::cout.flush();
};
//	��Ⱦ���ȿ��ӻ�2
inline void Visualize(int total, int index)
{
	static int cur = -1;
	if (cur == index)
	{
		return;
	}
	cur = index;
	int barWidth = 70;
	float progress = index / (float)total;
	std::cout << "[";
	int pos = barWidth * progress;
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) std::cout << "=";
		else if (i == pos) std::cout << ">";
		else std::cout << " ";
	}
	std::cout << "] [" << index << " / " << total << "] " << int(progress * 100.0f) << " %\r";
	std::cout.flush();
};