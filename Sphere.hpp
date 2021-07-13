#include "Object.hpp"
#include "Vector.hpp"
#include "Bounds3.hpp"
#include "Material.hpp"

class Sphere : public Object{
public:
    vec3 center;
    float radius, radius2;
    Material *m;
    float area;
    Sphere(const vec3 &c, const float &r, Material* mt = new Material()) : center(c), radius(r), radius2(r * r), m(mt), area(4 * PI *r *r) {}
    bool intersect(const Ray& ray) {
        // analytic solution
        vec3 L = ray.ori - center;
        float a = dot(ray.dir, ray.dir);
        float b = 2 * dot(ray.dir, L);
        float c = dot(L, L) - radius2;
        float t0, t1;
        float area = 4 * PI * radius2;
        if (!solveQuadratic(a, b, c, t0, t1)) return false;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
    }
    bool intersect(const Ray& ray, float &tnear, uint32_t &index) const
    {
        // analytic solution
        vec3 L = ray.ori - center;
        float a = dot(ray.dir, ray.dir);
        float b = 2 * dot(ray.dir, L);
        float c = dot(L, L) - radius2;
        float t0, t1;
        if (!
        solveQuadratic(a, b, c, t0, t1)) return false;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        tnear = t0;

        return true;
    }
    Intersection getIntersection(Ray ray){
        Intersection result;
        result.happened = false;
        vec3 L = ray.ori - center;
        float a = dot(ray.dir, ray.dir);
        float b = 2 * dot(ray.dir, L);
        float c = dot(L, L) - radius2;
        float t0, t1;
        if (!solveQuadratic(a, b, c, t0, t1)) return result;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return result;
        result.happened=true;

        result.coords = vec3(ray.ori + ray.dir * t0);
        result.normal = normalize(vec3(result.coords - center));
        result.m = this->m;
        result.obj = this;
        result.distance = t0;
        return result;

    }
    void getSurfaceProperties(const vec3 &P, const vec3 &I, const uint32_t &index, const vec2 &uv, vec3 &N, vec2 &st) const
    { N = normalize(P - center); }

    //vec3 evalDiffuseColor(const vec2 &st)const {
    //    return m->getColor();
    //}
    Bounds3 getBounds(){
        vec3 b1(center.x - radius, center.y - radius, center.z - radius);
        vec3 b2(center.x + radius, center.y + radius, center.z + radius);
        return Bounds3(b1, b2);
    }
    void Sample(Intersection &pos, float &pdf){
        float theta = 2.0 * PI * get_random_float(), phi = PI * get_random_float();
        vec3 dir(std::cos(phi), std::sin(phi)*std::cos(theta), std::sin(phi)*std::sin(theta));
        pos.coords = center + dir * radius;
        pos.normal = dir;
        pos.emit = m->getEmission();
        pdf = 1.0f / area;
    }
    float getArea(){
        return area;
    }
    bool hasEmit(){
        return m->hasEmission();
    }
};
