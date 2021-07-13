#pragma once
#include "Vector.hpp"
#include "Function.hpp"

enum MaterialType { DIFFUSE, MICROFACE};

class Material {
private:
    vec3 reflect(const vec3& I, const vec3& N) const {
        return I - N * 2 * dot(I, N);
    }

    // Compute refraction dir using Snell's law
    //
    // We need to handle with care the two possible situations:
    //
    //    - When the ray is inside the object
    //
    //    - When the ray is outside.
    //
    // If the ray is outside, you need to make cosi positive cosi = -N.I
    //
    // If the ray is inside, you need to invert the refractive indices and negate the normal N
    vec3 refract(const vec3& I, const vec3& N, const float& ior) const {
        float cosi = max(-1.0f, min(1.0f, dot(I, N)));
        float etai = 1, etat = ior;
        vec3 n = N;
        if (cosi < 0) { cosi = -cosi; }
        else { std::swap(etai, etat); n = -N; }
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? 0 : I * eta + n * (eta * cosi - sqrtf(k));
    }
    // Compute Fresnel equation
    //
    // \param I is the incident view dir
    //
    // \param N is the normal at the intersection point
    //
    // \param ior is the material refractive index
    //
    // \param[out] kr is the amount of light reflected
    void fresnel(const vec3& I, const vec3& N, const float& ior, float& kr) const {
        float cosi = max(-1.0f, min(1.0f, dot(I, N)));
        float etai = 1, etat = ior;
        if (cosi > 0) { std::swap(etai, etat); }
        // Compute sini using Snell's law
        float sint = etai / etat * sqrtf(max(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1) {
            kr = 1;
        }
        else {
            float cost = sqrtf(max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs + Rp * Rp) / 2;
        }
        // As a consequence of the conservation of energy, transmittance is given by:
        // kt = 1 - kr;
    }

    vec3 toWorld(const vec3& a, const vec3& N) {
        vec3 B, C;
        if (std::fabs(N.x) > std::fabs(N.y)) {
            float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
            C = vec3(N.z * invLen, 0.0f, -N.x * invLen);
        }
        else {
            float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
            C = vec3(0.0f, N.z * invLen, -N.y * invLen);
        }
        B = cross(C, N);
        return  B * a.x + C * a.y + N * a.z;
    }

public:
    MaterialType m_type;
    vec3 m_emission;
    vec3 Kd, Ks;

    float F;
    float ior;
    float roughness;//mircoface

    inline Material(MaterialType t = DIFFUSE, vec3 e = vec3(0.0f)):m_type(t), m_emission(e) {}

    inline vec3 getEmission() {
        return m_emission;
    }

    inline MaterialType getType() {
        return m_type;
    }

    inline bool hasEmission() {
        if (norm(m_emission) > EPSILON) {
            return true;
        }
        else {
            return false;
        }
    }

    // sample a ray by Material properties
    inline vec3 sample(const vec3& wi, const vec3& N) {
        // uniform sample on the hemisphere
        float x_1 = get_random_float(), x_2 = get_random_float();
        float z = std::fabs(1.0f - 2.0f * x_1);
        float r = std::sqrt(1.0f - z * z), phi = 2 * PI * x_2;
        vec3 localRay(r * std::cos(phi), r * std::sin(phi), z);
        return toWorld(localRay, N);
        //switch (m_type) {
        //case DIFFUSE: case MICROFACE:{
        //    // uniform sample on the hemisphere
        //    float x_1 = get_random_float(), x_2 = get_random_float();
        //    float z = std::fabs(1.0f - 2.0f * x_1);
        //    float r = std::sqrt(1.0f - z * z), phi = 2 * PI * x_2;
        //    vec3 localRay(r * std::cos(phi), r * std::sin(phi), z);
        //    return toWorld(localRay, N);

        //    break;
        //}
        //}
    }
    // given a ray, calculate the PdF of this ray
    inline float pdf(const vec3& wi, const vec3& wo, const vec3& N) {
        // uniform sample probability 1 / (2 * PI)
        if (dot(wo, N) > 0.0f)
            return 0.5f / PI;
        else
            return 0.0f;
        //switch (m_type) {
        //case DIFFUSE: case MICROFACE:{
        //    // uniform sample probability 1 / (2 * PI)
        //    if (dot(wo, N) > 0.0f)
        //        return 0.5f / PI;
        //    else
        //        return 0.0f;
        //    break;
        //}
        //}
    }
    // given a ray, calculate the contribution of this ray
    inline vec3 eval(const vec3& wi, const vec3& wo, const vec3& N) {
        switch (m_type) {
        case DIFFUSE: {
            // calculate the contribution of diffuse   model
            float cosalpha = dot(N, wo);
            if (cosalpha > 0.0f) {
                vec3 diffuse = Kd * eval_diffuse;
                return diffuse;
            }
            else
                return vec3(0.0f);
            break;
        }
        case MICROFACE: {
            float cosalpha = dot(N, wo);
            float cosbeta = dot(N, wi);
            if (cosalpha * cosbeta > 0.0f) {
                vec3 h = normalize(wi + wo);
                float fr = fresnelSchlick(wi, h, ior);
                float D = DistributionGGX(N, h, roughness);
                float G = GeometrySmith(N, wi, wo, roughness);

                float bsdf = fr * D * G / fabs(4 * dot(wi, N) * dot(wo, N));
                return Kd * bsdf;
            }
            else {
                return vec3(0);
                vec3 h(0.0f);
                float etai = 1;
                float etat = ior;

                if (cosalpha < 0) {
                    std::swap(etat, etai);
                }
                h = normalize(-(wi * etat + wo * etai));

                float fr = fresnelSchlick(wi, h, ior);
                float D = DistributionGGX(N, h, roughness);
                float G = GeometrySmith(N, wi, wo, roughness);

                float OdotH = dot(wo, h);
                float IdotH = dot(wi, h);
                float sqrtDenom = OdotH * etai + etat * IdotH;
                
                float factor = fabsf(OdotH * IdotH / (dot(N, wi) * dot(N, wo)));
                float bsdf = factor * ((1 - fr) * D * G * etat * etai) / (sqrtDenom * sqrtDenom);
                return Kd * bsdf;
            }
            break;
        }
        }
    }

    float DistributionGGX(const vec3& N, const vec3& H, float a) {
        float a2 = a * a;
        float NdotH = max(dot(N, H), 0.0f);
        float NdotH2 = NdotH * NdotH;

        float nom = a2;
        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / denom;
    }
    float GeometrySchlickGGX(float NdotV, float k) {
        float nom = NdotV;
        float denom = NdotV * (1.0 - k) + k;

        return nom / denom;
    }
    float GeometrySmith(vec3 N, vec3 V, vec3 L, float k) {
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float ggx1 = GeometrySchlickGGX(NdotV, k);
        float ggx2 = GeometrySchlickGGX(NdotL, k);

        return ggx1 * ggx2;
    }
    float fresnelSchlick(const vec3& v, const vec3& h, const float& ior) {
        float cosTheta = dot(v, h);
        float R0 = pow((ior - 1) / (ior + 1), 2);
        return R0 + (1 - R0) * pow((1 - cosTheta), 5);
    }

    vec3 ggxSample(const vec3& wo, const vec3& N,vec3& wi, float& pdf) {
        float a = roughness;
        float a2 = a * a;

        float e0 = get_random_float();
        float e1 = get_random_float();
        float cos2Theta = (1 - e0) / (e0 * (a2 - 1) + 1);
        float cosTheta = sqrt(cos2Theta);
        float sinTheta = sqrt(1 - cos2Theta);
        float phi = 2 * PI * e1;
        vec3 localdir(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
        vec3 h = toWorld(localdir, N);


        float fr = fresnelSchlick(wo, h, ior);
        bool isReflect = get_random_float() < fr;
        if (isReflect) {
            wi = h * 2.0f * dot(wo, h) - wo;
            if (dot(wo, N) * dot(wi, N) <= 0) {
                pdf = 0;
                return vec3(0);
            }

            float D = DistributionGGX(N, h, roughness);
            pdf = fr * D * dot(h, N) / (4 * (fabs(dot(wo, h))));
            float G = GeometrySmith(N, wi, wo, roughness);
            float bsdf = fr * D * G / fabs(4.0 * dot(N, wi) * dot(N, wo));

            return Kd * bsdf;
        }
        else {
            pdf = 0;
            return vec3(0);
            float etai = 1.f, etat = ior;
            wi = refract(-wo, h, ior);
            if (dot(wo, N) < 0.f) {
                std::swap(etai, etat);
            }
            if (dot(wo, N) * dot(wi, N) > 0.0f || norm(wi) == 0) {
                pdf = 0.0f;
                return vec3(0.0f);
            }
            float Dh = DistributionGGX(N, h, roughness);
            float HoWo = dot(h, wo);
            float HoWi = dot(h, wi);
            float sqrtDenom = etai * HoWo + etat * HoWi;
            float dwh_dwi = (etat * etat * fabsf(HoWi)) / (sqrtDenom * sqrtDenom);
            pdf = Dh * dot(h, N) * dwh_dwi * (1 - fr);

            float Gwowih = GeometrySmith(N, wi, wo, roughness);
            float factor = fabsf(HoWo * HoWi / (dot(N, wi) * dot(N, wo)));
            float bsdf = factor * ((1 - fr) * Dh * Gwowih * etat * etat) /
                (sqrtDenom * sqrtDenom);
            return Kd * bsdf;
        }
    }
};