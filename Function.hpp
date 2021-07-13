#pragma once
#include <windows.h>
#include <iostream>
#include <cassert>
#include <random>
#include <string>
#include <vector>
#include <cmath>
#include <ctime>

#include "Vector.hpp"

const float PI = 3.14159265359f;
const float EPSILON = 0.0001;
const float eval_diffuse = 1 / PI;

inline float get_random_float() {
    static std::random_device dev;
    static std::mt19937 rng(dev());
    static std::uniform_real_distribution<float> dist(0.f, 1.f); // distribution in range [1, 6]

    return dist(rng);
}
//  光线与球相交
inline bool solveQuadratic(const float& a, const float& b, const float& c, float& x0, float& x1) {
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) x0 = x1 = -0.5 * b / a;
    else {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discr)) :
            -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1) std::swap(x0, x1);
    return true;
}

inline vec3 lerp(const vec3& a, const vec3& b, const float& t) {
    return a * (1 - t) + b * t;
}
inline float dot(const vec3& u, const vec3& v) {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}
inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(
        u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x);
}

inline float norm(const vec3& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline vec3 normalize(const vec3& u) {
    float n = std::sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
    return vec3(u.x / n, u.y / n, u.z / n);
}
inline float radians(float f) {
    return PI * f / 180;
}

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
    std::cout << "] [" << index << " / " << total << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
};
inline void Visualize(float progress)
{
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
};

bool rayTriangleIntersect(const vec3& v0, const vec3& v1,
    const vec3& v2, const vec3& ori,
    const vec3& dir, float& tnear, float& u, float& v) {
    //判断光线是否与三角面相交
    // ray = O + tD
    // (1 - u - v) * v0 + u * v1 + v * v2 = O + tD ,u >= 0,v >= 0
    //  t                       S2 * E2     S = O - v0    |  S1 = D cross E2
    //  u   =   1/(S1 * E1)  *  S1 * S      E1 = v1 - v2  |  S2 = S cross E1
    //  v                       S2 * D      E2 = v2 - v0  |
    vec3 E1 = v1 - v0;
    vec3 E2 = v2 - v0;
    vec3 S1 = cross(dir, E2);
    float det = dot(E1, S1);
    if (det <= 0) {
        return false;
    }

    vec3 S = ori - v0;
    u = dot(S, S1);
    if (u < 0 || u > det) {
        return false;
    }

    vec3 S2 = cross(S, E1);
    v = dot(dir, S2);
    if (v < 0 || u + v > det) {
        return false;
    }

    float invDet = 1 / det;

    tnear = dot(E2, S2) * invDet;
    u *= invDet;
    v *= invDet;

    return true;
}

