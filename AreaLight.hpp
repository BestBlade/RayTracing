#pragma once
#include "Light.hpp"
#include "Vector.hpp"
#include "Function.hpp"
//√Êπ‚‘¥
class AreaLight : public Light {
public:
    float length;
    vec3 normal;
    vec3 u;
    vec3 v;

    AreaLight(const vec3& p, const vec3& i) : Light(p, i) {
        normal = vec3(0, -1, 0);
        u = vec3(1, 0, 0);
        v = vec3(0, 0, 1);
        length = 100;
    }

    vec3 SamplePoint() const {
        auto randU = get_random_float();
        auto randV = get_random_float();
        return position + u * randU + v * randV;
    }
};