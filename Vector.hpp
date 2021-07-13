#pragma once
#include <iostream>
#include <cmath>
#include <algorithm>

class vec3 {
public:
    float x, y, z;
    vec3() : x(0), y(0), z(0) {}
    vec3(float xx) : x(xx), y(xx), z(xx) {}
    vec3(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
    //常对象只能调用常函数，避免进行成员变量的修改
    vec3 operator * (const float& r) const {
        return vec3(x * r, y * r, z * r);
    }
    vec3 operator / (const float& r) const {
        return vec3(x / r, y / r, z / r);
    }

    vec3 operator * (const vec3& v) const {
        return vec3(x * v.x, y * v.y, z * v.z);
    }
    vec3 operator - (const vec3& v) const {
        return vec3(x - v.x, y - v.y, z - v.z);
    }
    vec3 operator + (const vec3& v) const {
        return vec3(x + v.x, y + v.y, z + v.z);
    }
    vec3 operator - () const {
        return vec3(-x, -y, -z);
    }
    vec3& operator += (const vec3& v) {
        x += v.x, y += v.y, z += v.z;
        return *this;
    }
    float operator[](int index) const {
        return (&x)[index];
    };
};

class vec2{
public:
    float x, y;

    vec2() : x(0), y(0) {}
    vec2(float xx, float yy) : x(xx), y(yy) {}
    vec2 operator * (const float& r) const {
        return vec2(x * r, y * r);
    }
    vec2 operator / (const float& r) const {
        return vec2(x / r, y / r);
    }
    vec2 operator * (const vec2& v) const {
        return vec2(x * v.x, y * v.y);
    }
    vec2 operator - (const vec2& v) const {
        return vec2(x - v.x, y - v.y);
    }
    vec2 operator + (const vec2& v) const {
        return vec2(x + v.x, y + v.y);
    }
    vec2 operator - () const {
        return vec2(-x, -y);
    }
    vec2& operator += (const vec2& v) {
        x += v.x, y += v.y;
        return *this;
    }
};