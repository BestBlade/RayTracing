#pragma once
#include "Function.hpp"
#include "Intersection.hpp"
#include "Bounds3.hpp"
#include "Ray.hpp"

class Object {
public:
    Object() {}
    virtual ~Object() {}
    virtual bool intersect(const Ray& ray, float&, uint32_t&) const = 0;
    virtual Intersection getIntersection(Ray _ray) = 0;
    virtual void getSurfaceProperties(const vec3&, const vec3&, const uint32_t&, const vec2&, vec3&, vec2&) const = 0;
    virtual Bounds3 getBounds() = 0;
    virtual float getArea() = 0;
    virtual void Sample(Intersection& pos, float& pdf) = 0;
    virtual bool hasEmit() = 0;
};