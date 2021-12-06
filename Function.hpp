#pragma once
#include <cmath>
#include <random>
#include <iostream>

const float EPS = 1e-3;
const float PI = 3.1415926535;
const float PI_INV = 1 / PI;
//	快速求1/√x，注意没有考虑x<0的情况
inline float mysqrtinv(float x) {
	if (x == 0) {
		return 0;
	}
	float xhalf = 0.5f * x;
	int i = *(int*)&x;
	i = 0x5f3759df - (i >> 1);
	x = *(float*)&i;
	x = x * (1.5f - xhalf * x * x);
	return x;
}
//	快速求√x，注意没有考虑x<0的情况
inline float mysqrt(float x) {
	if (x == 0) {
		return 0;
	}
	return 1.f / mysqrtinv(x);
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
