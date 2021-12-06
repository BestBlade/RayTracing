#pragma once
#include "Vector.hpp"
//	定义材质属性
enum MaterialType {DIFFUSE,MICROFACET};
//	材质类
class Material {
private:
	//	计算反射公式，注意方向
	vec3 reflect(const vec3& V, const vec3& N) const{
		//	↖V	  ↑N	  ↗L
		//	  ↖	  ↑	↗
		//	    ↖↑↗
		//		inter
		return N * 2 * V.dot(N) - V;
	}
	//	菲涅尔项，可以用ior代替F0，公式在注释里
	float fresnel(const vec3& I, const vec3& N, const float& F0) const{
		// F0 = (n1-n2)^2 / (n1+n2)^2	反射率
		// T0 = 1-F0	透射率
		
		//FSchlick(h,v,F0)=F0 + (1 − F0) * (1 − (n dot v)) ^ 5
		return F0 + (1 - F0) * pow((1 - N.dot(I)), 5);
	}
	//	vec3的F0，没用到，程序里用的float的F0
	vec3 fresnel(const vec3& I, const vec3& N, const vec3& F0) const{
		//FSchlick(h,v,F0)=F0 + (1 − F0) * (1 − (n dot v)) ^ 5
		return F0 + (vec3(1.f) - F0) * pow((1 - N.dot(I)), 5);
	}
	//	折射，暂不考虑，暂不计算透明物体（其实还没研究怎么做
	float refract(const vec3& I, const vec3& N, const float& ior) const {
		float F0 = (ior - 1) * (ior - 1) / ((ior + 1) * (ior + 1));
		float T0 = 1.f - F0;
	}
	//	局部坐标转化为世界坐标
	vec3 toWorld(const vec3& u, const vec3& N) const{
		vec3 B, C;
		if (abs(N.x) > abs(N.y)) {
			float invLen = mysqrtinv(N.x * N.x + N.z * N.z);
			C = vec3(N.z * invLen, 0.f, -N.x * invLen);
		}
		else {
			float invLen = mysqrtinv(N.y * N.y + N.z * N.z);
			C = vec3(0.f, N.z * invLen, -N.y * invLen);
		}
		B = C.cross(N);
		return B * u.x + C * u.y + N * u.z;
	}
	//	D(h)的概率分布，计算半程向量H在宏观法线N和roughness参数下的概率
	float DistributionGGX(const vec3& N, const vec3& H, float a) {
		// D = a ^ 2
		//	   ---------------------
		//	   pi * ( (N*H)^2 * (a ^ 2 - 1) + 1) ^ 2
		// a = roughness * roughness
		float a2 = a * a;
		float NdotH = std::max(0.f, N.dot(H));
		float NdotH2 = NdotH * NdotH;

		float denom = (NdotH2 * (a2 - 1.f) + 1.f);
		denom = PI * denom * denom;
		return a2 / denom;
	}
	//	自遮挡项G1
	float GeometrySchlickGGX(float NdotV, float a) {
		float denom = NdotV * (1.f - a) + a;
		return NdotV / denom;
	}
	//	G(V,L,N) = G1(V,N) * G1(L,N),借鉴了unreal的计算公式
	float GeometrySmith(const vec3& wi, const vec3& wo, const vec3& N,float roughness) {
		float NdotL = N.dot(wo);
		float NdotV = N.dot(wi);
		float k = (roughness + 1) * (roughness + 1) / 8;

		return GeometrySchlickGGX(NdotL, k) * GeometrySchlickGGX(NdotV, k);
	}
public:
	MaterialType materialType;	//	材质类型
	float F0;					//	反射率F0
	float roughness;			//	粗糙度roughness
	vec3 materialEmission;		//	材料发光强度
	vec3 Kd;					//	材料漫反射系数（其实是颜色
	vec3 Ks;

	Material(MaterialType mT = DIFFUSE, vec3 mE = vec3(0.f), float f0 = 0.9, float a = 0.01)
		:materialType(mT), materialEmission(mE), F0(f0), roughness(a) {}	//	默认构造函数，DIFFUSE情况下不会使用f0和roughess进行计算
	//	获取材质属性
	inline MaterialType getType() {
		return materialType;
	}
	//	获取发光属性
	inline vec3 getEmission() {
		return materialEmission;
	}
	//	判断是否发光
	inline bool hasEmission() {
		return materialEmission.norm() > EPS ? true : false;
	}
	//	对光线的出射方向进行采样
	inline vec3 sample(const vec3& wi, const vec3& N) {
		//上半球采样取半程向量H
		//float u = getrandom();
		//float v = getrandom();
		//float phi = v * 2 * PI;
		//float costheta = 1.f - u;
		//float sintheta = mysqrt(1 - costheta * costheta);
		//vec3 localRay(cos(phi) * sintheta, sin(phi) * sintheta, costheta);
		//return toWorld(localRay, N);
		vec3 localRay;
		if (materialType == DIFFUSE) {
			// 漫反射情况下，对球面均匀（uniform）采样取反射方向L
			float u = getrandom();
			float v = getrandom();
			float theta = 2 * PI * u;
			float cosphi = 1.f - 2 * v;
			float sinphi = mysqrt(1 - cosphi * cosphi);
			localRay = vec3(cosphi * sin(theta), sinphi * sin(theta), cos(theta));
			//	还原到世界坐标
			return toWorld(localRay, N);
		}
		else if (materialType == MICROFACET) {
			//	微表面模型下，使用Cosine-Weighted半球面采样半程向量H
			float u = getrandom();
			float v = getrandom();
			float a = roughness * roughness;
			float phi = 2 * u * PI;
			float cos2theta = (1 - v) / (1 + v * (a * a - 1));
			float costheta = std::min(1.0f,mysqrt(cos2theta));
			float sintheta = mysqrt(1 - cos2theta);
			vec3 H(sintheta * cos(phi), sintheta * sin(phi), costheta);
			//	将H转换为世界坐标
			H = toWorld(H, N);
			//	根据入射wi和半程向量H求反射方向L
			localRay = reflect(wi, H);
			//	返回最终的反射方向L
			return localRay;
		}
	}

	inline float pdf(const vec3& wi, const vec3& wo, const vec3& N) {
		if (materialType == DIFFUSE) {
			if (wo.dot(N) > 0) {
				//	发生了反射的情况下，pdf = 1/2PI
				return 0.5f * PI_INV;
			}
			else {
				//	反射没有发生
				return 0.f;
			}
		}
		else if (materialType == MICROFACET) {
			vec3 H = (wi + wo).normalized();
			float NdotL = N.dot(wo);
			float NdotV = N.dot(wi);
			//	如果乘积大于零说明是反射
			if (NdotL * NdotV > 0.f) {
				//	此时pdf = D*dot(N,H)/(4*dot(H,V)),根据论文来的
				float D = DistributionGGX(N, H, roughness * roughness);	//	UE4里D项的a为roughness * roughness
				float pdf = D * N.dot(H) / abs(4.f * H.dot(wi));
				return pdf;
			}
			else {
				//	折射，暂不考虑
				return 0.f;
			}
		}
	}

	inline vec3 eval(const vec3& wi, const vec3& wo, const vec3& N) {
		if (materialType == DIFFUSE) {
			if (wo.dot(N) > EPS) {
				//	接触点的颜色贡献为 Kd/PI
				return Kd * PI_INV;
			}
			else {
				return vec3(0.f);
			}
		}
		else if (materialType == MICROFACET) {
			float NdotV = N.dot(wi);
			float NdotL = N.dot(wo);

			//	反射
			if (NdotV * NdotL > 0.f) {
				vec3 H = (wi + wo).normalized();
				float F = fresnel(wi, H, F0);
				float D = DistributionGGX(N, H, roughness * roughness);		//	应该和156行的D计算方法一致，统一为roughness^2
				float G = GeometrySmith(wi, wo, N, roughness);
				//	微表面模型下高光项为 DFG/(4*dot(N,V)*dot(N,L))
				float specular = D * F * G / (4.f * NdotV * NdotL);
				return Kd * specular;
			}
			//	折射
			else {
				//	暂时不计算折射
				return vec3(0.f);
			}
		}
	}
};
