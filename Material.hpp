#pragma once
#include "Vector.hpp"
//	定义材质属性
enum MaterialType {DIFFUSE,MICROFACET};
//	材质类
class Material {
private:
	//	局部坐标转化为世界坐标
	vec3 toWorld(const vec3& u, const vec3& N) const{
		vec3 B, C;
		if (abs(N.x) > abs(N.y)) {
			float Len = mysqrt(N.x * N.x + N.z * N.z);
			C = vec3(N.z / Len, 0.f, -N.x / Len);
			//float invLen = mysqrtinv(N.x * N.x + N.z * N.z);
			//C = vec3(N.z * invLen, 0.f, -N.x * invLen);
		}
		else {
			float Len = mysqrt(N.y * N.y + N.z * N.z);
			C = vec3(0.f, N.z / Len, -N.y / Len);
			//float invLen = mysqrtinv(N.y * N.y + N.z * N.z);
			//C = vec3(0.f, N.z * invLen, -N.y * invLen);
		}
		B = C.cross(N);
		return B * u.x + C * u.y + N * u.z;
	}
	//	D(h)的概率分布，计算半程向量H在宏观法线N和roughness参数下的概率
	float DistributionGGX(const vec3& N, const vec3& H, float roughness) {
		////	Learning OPENGL跟下面的相同
		////	这个公式会在roughness过低的时候denom被计算为0，导致D为inf
		//float a = roughness * roughness;
		//float a2 = a * a;
		//float NdotH = N.dot(H);
		//if (NdotH < 0.f) {
		//	return 0.f;
		//}
		//float NdotH2 = NdotH * NdotH;
		//float denom = (NdotH2 * (a2 - 1) + 1);
		//denom = PI * denom * denom;
		//return a2 / denom;

		////	https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
		////	式(33)
		float a = roughness * roughness;
		float a2 = a * a;
		float NdotH = N.dot(H);
		if (NdotH < 0.f) {
			return 0.f;
		}
		float nom = a2;
		float cos2theta = NdotH * NdotH;
		float tan2theta = 1 / cos2theta - 1;
		float denom = cos2theta * (a2 + tan2theta);
		denom = PI * denom * denom;
		return nom / denom;
	}
	//	自遮挡项G1
	float GeometrySchlickGGX(const vec3& V, const vec3& N, const vec3& H, float a) {
		//	https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
		//	式(34)
		float NdotV = N.dot(V);
		float tan2theta = 1 / (NdotV * NdotV) - 1;
		if (NdotV * H.dot(V) < 0.f) {
			return 0.f;
		}
		if (tan2theta < EPS) {
			return 1.f;
		}
		float root = a * a * tan2theta;
		return 2 / (1 + mysqrt(1 + root));
	}
	//	G(V,L,N) = G1(V,N) * G1(L,N),借鉴了unreal的计算公式
	float GeometrySmith(const vec3& V, const vec3& L, const vec3& N, const vec3& H, float roughness) {
		//	https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
		//	式(7)式(34)
		if (N.dot(V) * H.dot(V) < 0.f || N.dot(L) * H.dot(L) < 0.f) {
			return 0.f;
		}
		float a = roughness * roughness;
		return GeometrySchlickGGX(V, N, H, a) * GeometrySchlickGGX(L, N, H, a);
	}
public:
	bool isMetal;
	MaterialType materialType;	//	材质类型
	float F0;					//	反射率F0
	float ior;
	float roughness;			//	粗糙度roughness
	vec3 materialEmission;		//	材料发光强度
	vec3 baseColor;				//	材料漫反射系数（其实是颜色
	vec3 Ks;

	Material(MaterialType mT = DIFFUSE, vec3 mE = vec3(0.f), bool isM = true, float _ior = 1.333f, float f0 = 0.9f, float a = 0.01f)
		:materialType(mT), materialEmission(mE), F0(f0), roughness(a),ior(_ior),isMetal(isM) {
		if (!isMetal) {
			F0 = std::pow((ior - 1) / (ior + 1), 2);
		}
	}	//	默认构造函数，DIFFUSE情况下不会使用f0和roughess进行计算
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

	//	计算反射公式，注意方向
	vec3 reflect(const vec3& V, const vec3& N) const {
		//	↖V	  ↑N	  ↗L
		//	  ↖	  ↑	↗
		//	    ↖↑↗
		//		inter

		return N * 2 * V.dot(N) - V;
	}
	//	菲涅尔项，可以用ior代替F0，公式在注释里
	float fresnel(const vec3& V, const vec3& N, const float& ior) const {
		//	F0 = (n1-n2)^2 / (n1+n2)^2	反射率
		//	T0 = 1 - F0	透射率

		//	FSchlick(h,v,F0)=F0 + (1 − F0) * (1 − (n dot v)) ^ 5
		float costheta = N.dot(V);
		if (costheta < 0.f) {
			//	从光密到光疏介质,计算折射方向
			vec3 next = refract(-V, N);
			if (next.norm() < EPS) {
				//	没有发生折射。即全反射
				return 1.f;
			}
			else {
				//	光路可逆，相当于从next入射计算F
				costheta = N.dot(next);
			}
		}
		return F0 + (1 - F0) * pow5(1 - costheta);
	}
	//	vec3的F0，没用到，程序里用的float的F0
	vec3 fresnel(const vec3& I, const vec3& N, const vec3& F0) const {
		//FSchlick(h,v,F0)=F0 + (1 − F0) * (1 − (n dot v)) ^ 5
		return F0 + (vec3(1.f) - F0) * pow5(1 - N.dot(I));
	}
	//	折射，I为eye到inter，注意方向，与V是相反的
	vec3 refract(const vec3& I, const vec3& N) const {
		//float T0 = 1.f - F0;
		float cosi = std::max(-1.f, std::min(1.f, I.dot(N)));
		float etai = 1, etao = ior;
		vec3 n = N;
		if (cosi < 0.f) { 
			//	从空气入射etai = 1,etao = ior
			cosi = -cosi; 
		}
		else { 
			//	从介质中射入空气etai = ior，etao = 1
			std::swap(etai, etao); 
			n = -N; 
		}
		float eta = etai / etao;
		float k = 1 - eta * eta * (1 - cosi * cosi);

		return k < 0 ? 0 : I * eta + n * (eta * cosi - mysqrt(k));
	}
	//	对光线的半程向量H进行采样
	inline vec3 sample(const vec3& wi, const vec3& N) {
		vec3 localRay;
		if (materialType == DIFFUSE) {
			//	上半球均匀采样取半程向量H
			float u = getrandom();
			float v = getrandom();
			float phi = v * 2 * PI;
			float costheta = 1.f - u;
			float sintheta = mysqrt(1 - costheta * costheta);
			vec3 localRay(cos(phi) * sintheta, sin(phi) * sintheta, costheta);
			return toWorld(localRay, N);
			//// 漫反射情况下，对球面均匀（uniform）采样取反射方向L
			//float u = getrandom();
			//float v = getrandom();
			//float theta = 2 * PI * u;
			//float cosphi = 1.f - 2 * v;
			//float sinphi = mysqrt(1 - cosphi * cosphi);
			//localRay = vec3(cosphi * sin(theta), sinphi * sin(theta), cos(theta));
			////	还原到世界坐标
			//return toWorld(localRay, N);
		}
		else if (materialType == MICROFACET) {
			//	微表面模型下，使用Cosine-Weighted半球面采样半程向量H
			//	https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
			//	式(35)式(36)
			float u = getrandom();
			float v = getrandom();
			float a = roughness * roughness;
			float phi = 2 * u * PI;
			float cos2theta = (1 - v) / (1 + v * (a * a - 1));
			float costheta = std::min(1.0f,mysqrt(cos2theta));
			float sintheta = mysqrt(1 - cos2theta);
			vec3 localRay(sintheta * cos(phi), sintheta * sin(phi), costheta);
			return toWorld(localRay, N);
		}
	}

	inline float pdf(const vec3& V, const vec3& L, const vec3& N,const vec3& H) {
		if (materialType == DIFFUSE) {
			if (N.dot(L) > 0) {
				//	发生了反射的情况下，pdf = 1/2PI
				return 0.5f * PI_INV;
			}
			else {
				//	反射没有发生
				return 0.f;
			}
		}
		else if (materialType == MICROFACET) {
			float NdotL = N.dot(L);
			float NdotV = N.dot(V);
			float F = fresnel(V, H, F0);

			//	如果乘积大于零说明是反射
			if (NdotL * NdotV > 0.f) {
				//	此时pdf = D*dot(N,H)/(4*dot(H,V)),根据论文来的
				float D = DistributionGGX(N, H, roughness);
				float pdf = D * N.dot(H) / abs(4.f * H.dot(V));
				return pdf;
			}
			else {
				//	折射
				if (isMetal || L.norm() < EPS) {
					//	金属没有折射,或者没有发生折射
					return 0.f;
				}
				float etai = 1.f;
				float etao = ior;
				float NdotL = N.dot(L);
				float NdotV = N.dot(V);
				if (NdotV < 0.f) {
					//	光线从内部向空气折射出
					//	此时etain是物体的ior，etaout是空气的ior为1
					std::swap(etai, etao);
				}
				float HdotL = H.dot(L);
				float HdotV = H.dot(V);
				//	入射ior*入射夹角 + 出射ior*出射夹角
				float denom = std::pow(etai * HdotV + etao * HdotL, 2);
				float dHdL = (etao * etao * abs(HdotL)) / denom;
				float D = DistributionGGX(N, H, roughness);
				float pdf = D * N.dot(H) * dHdL;
				return pdf;
			}
		}
	}

	inline vec3 eval(const vec3& V, const vec3& L, const vec3& N,const vec3& H) {
		if (materialType == DIFFUSE) {
			if (L.dot(N) > EPS) {
				//	接触点的颜色贡献为 Kd/PI
				return baseColor * PI_INV;
			}
			else {
				return vec3(0.f);
			}
		}
		else if (materialType == MICROFACET) {
			float NdotV = N.dot(V);
			float NdotL = N.dot(L);
			//	反射
			if (NdotV * NdotL > 0.f) {
				float F = fresnel(V, H, F0);
				float D = DistributionGGX(N, H, roughness);
				float G = GeometrySmith(V, L, N, H, roughness);
				//	微表面模型下高光项为 DFG/(4*dot(N,V)*dot(N,L))
				float brdf = D * F * G / (4.f * NdotV * NdotL);
				return baseColor * brdf;
			}
			//	折射
			else {
				if (isMetal || L.norm() < EPS) {
					//	金属不折射
					return vec3(0.f);
				}
				//	https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
				//	式(21)
				float etai = 1;
				float etao = ior;
				if (N.dot(V) < 0.f) {
					//	光线从内部向空气折射出
					//	此时etain是物体的ior，etaout是空气的ior为1
					std::swap(etai, etao);
				}
				float HdotL = H.dot(L);
				float HdotV = H.dot(V);
				//	入射ior*入射夹角 + 出射ior*出射夹角
				float denom = etai * HdotV + etao * HdotL;
				denom *= denom;
				float dHdL = (etao * etao * abs(HdotL)) / denom;


				float F = fresnel(V, H, F0);
				float D = DistributionGGX(N, H, roughness);
				float G = GeometrySmith(V, L, N, H, roughness);


				float factor = abs(HdotL * HdotV / (N.dot(V) * N.dot(L)));
				float btdf = factor * (1 - F) * D * G * etao * etao / denom;
				return baseColor * btdf;
			}
		}
	}
	//	castRay1()函数用↓
	vec3 caculateBSDF(const vec3& V, const vec3& N,const vec3 & H, vec3& L, float& pdf) {
		//vec3 H = sample(V, N);
		if (materialType == DIFFUSE) {
			L = reflect(V, H);
			float NdotV = N.dot(V);
			float NdotL = N.dot(L);

			if (NdotL * NdotV < 0.f) {
				pdf = 0.f;
				return vec3(0.f);
			}
			else {
				pdf = 0.5 * PI_INV;
				return baseColor * PI_INV;
			}
		}
		else {
			float F = fresnel(V, H, F0);
			if (isMetal || getrandom() < F) {
				//	反射
				L = reflect(V, H);
				float NdotL = N.dot(L);
				float NdotV = N.dot(V);

				if (NdotL * NdotV < 0.f) {
					pdf = 0.f;
					return vec3(0.f);
				}

				float D = DistributionGGX(N, H, roughness);
				float G = GeometrySmith(V, L, N, H, roughness);

				pdf = D * N.dot(H) / abs(4.f * H.dot(V));

				float brdf = D * F * G / (4.f * NdotV * NdotL);

				return baseColor * brdf;
			}
			else {
				//	折射
				float etai = 1.f;
				float etao = ior;
				L = refract(-V, H);

				float NdotL = N.dot(L);
				float NdotV = N.dot(V);
				if (NdotV < 0.f) {
					std::swap(etai, etao);
				}

				if (NdotL * NdotV > 0.f || L.norm() < EPS) {
					pdf = 0.f;
					return vec3(0.f);
				}

				//vec3 H = (wi + wo).normalized();
				float HdotL = H.dot(L);
				float HdotV = H.dot(V);

				//	入射ior*入射夹角 + 出射ior*出射夹角
				float denom = std::pow(etai * HdotV + etao * HdotL, 2);
				float dHdL = (etao * etao * abs(HdotL)) / denom;

				float D = DistributionGGX(N, H, roughness);
				float G = GeometrySmith(V, L, N, H, roughness);

				float factor = abs(HdotL * HdotV / (NdotL * NdotV));

				pdf = D * N.dot(H) * dHdL;

				float btdf = factor * (1 - F) * D * G * etao * etao / denom;

				return baseColor * btdf;
			}
		}
	}
};