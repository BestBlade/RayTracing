#pragma once
#include <cmath>
#include <random>
#include <iostream>
#include <cassert>

const float EPS = 1e-6;
const float PI = 3.1415926535;
const float PI_INV = 1 / PI;
float mysqrt(float x);
//	快速求1/√x，注意没有考虑x<0的情况，但是精度不够
inline float mysqrtinv(float x) {
	//	https://www.cnblogs.com/signal/p/3818332.html
	float xhalf = 0.5f * x;
	int i = *(int*)&x;
	i = 0x5f37642f - (i >> 1);	// 计算第一个近似根
	x = *(float*)&i;
	x = x * (1.5f - xhalf * x * x);	// 牛顿迭代法
	return x;
}
//	快速求√x，注意没有考虑x<0的情况
inline float mysqrt(float x) {
	//	https://blog.csdn.net/xtlisk/article/details/51249371
	//	标准库的是准的,但是和球求交会产生奇怪的bug
	//return std::sqrtf(x);
	//	计算数值偏小
	//return 1 / mysqrtinv(x);	//1/√x
	//	计算数值偏大
	//return x * mysqrtinv(x);	//x*√x

	//	计算数值偏大
	//	x86限定
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

	//	计算数值偏小
	//	这个精度也可以接受
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
//	角度转弧度
inline float radians(float degree) {
	return degree * PI / 180;
}
//	获取0-1随机数，使用static优化性能
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
//	渲染进度可视化1
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
//	渲染进度可视化2
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