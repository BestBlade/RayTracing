#pragma once
#include "BVHTree.hpp"
#include "AreaLight.hpp"
#include "Function.hpp"

class Scene {
public:
	unsigned int width;
	unsigned int height;
	float fov;
	float RussianRoulette;
	BVHAccel* BVH;

	std::vector<Object*> objects;
	std::vector<std::unique_ptr<Light>> lights;

	Scene(unsigned int w, unsigned int h) :width(w), height(h) {}
	//	添加物体
	void Add(Object* obj) {
		objects.emplace_back(obj);
	}
	//	添加灯光（没用到）
	void Add(std::unique_ptr<Light> light) {
		lights.emplace_back(std::move(light));
	}
	//	创建场景BVH
	void buildBVH() {
		std::cout << "[!]Generating BVH...\n\n";
		BVH = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
	}

	//	获取采样点属性，并且返回采样点pdf,只有light用
	float SampleLight(Intersection& inter) const{
		float emitAreaSum = 0;
		for (unsigned int i = 0; i < objects.size(); ++i) {
			if (objects[i]->hasEmit()) {
				emitAreaSum += objects[i]->getArea();
			}
		}

		float p = getrandom() * emitAreaSum;
		emitAreaSum = 0;
		float pdf;
		for (unsigned int i = 0; i < objects.size(); ++i) {
			if (objects[i]->hasEmit()) {
				emitAreaSum += objects[i]->getArea();
				if (p < emitAreaSum) {
					pdf = objects[i]->Sample(inter);
					break;
				}
			}
		}
		return pdf;
	}
	//	计算光线与场景交点
	Intersection getIntersection(const Ray& ray)const {
		return BVH->getIntersection(ray);
	}


	vec3 castRay(const Ray& ray, unsigned int depth) const{
		// TO DO Implement Path Tracing Algorithm here
		//  eye					light
		//    ↖               ↗ ↓ light.norm
		//   -V ↖           ↗ L
		//        ↖   N   ↗
		//          ↖ ↑ ↗
		//            pos
		//  ray = o + td
		//  if the ray landed on the object,there's a direct contact point : pos
		//  if the pos is bright,return the emission of this pos
		//  else,randomly take a light from light L_i and get its' pdf with function [sampleLight(L_i, pdf)]
		//  L_i is a bright place.not just a light

		//  then distance = (L_i.postion - pos.postion)^2
		//  wo = (L_i.postion-pos.postion).norm()
		//  wi = ray.dir
		//  cos_theta = N * wo
		//  cos_theta_x = -wo * n'
		//  f_r = pos.m->eval(wi,wo,N)
		//  L_dir = L_i.emission * f_r * cos_theta * cos_theta_x / distance / pdf

		//  if the random num > RR,stop caculate the reflect from other place(except light)
		//  caculate L_indir
		//  eye                 pos2
		//    ↖               ↗
		//   -V ↖           ↗indirL
		//        ↖   N   ↗
		//          ↖ ↑ ↗
		//            pos
		//  wi_ = wi
		//  here,use [pos.m->sample(wi_,N)] to get a wo_
		//  the reflectray = {pos,wo_}
		//  caculate pos2 with intersect(reflectray)
		//  the pos2 can't be a light so [pos2.m->hasEmission] should be [false]
		//  the brdf = pos.m->eval(wi_,wo_,N)
		//  cos_theta_ = N * wo_
		//  pdf_ = pos.m.pdf(wi,N)
		//  pos2.emssion = castray(reflectray,depth+1)
		//  L_indir = pos2.emssion * brdf * cos_theta_ / pdf_ / RR

		//	将结果规整，防止出现负数
		auto format = [&](vec3& a) {
			if (a.x < 0) a.x = 0;
			if (a.y < 0) a.y = 0;
			if (a.z < 0) a.z = 0;
		};
		//	如果从相机来的光线没办法和场景接触，说明没看到东西
		Intersection inter = getIntersection(ray);
		if (!inter.happened) {
			return vec3(0.f);
		}

		if (inter.m->hasEmission()) {
			return inter.emit;
		}
		//	如果看到了灯光，但不是直接看到的(depth!=0),就不管,加速收敛的trick,但是会导致没有灯光的反射
		//if (inter.m->hasEmission()) {
		//	return depth == 0 ? inter.emit : vec3(0.f);
		//}

		Intersection lightpos;
		float lightpdf = SampleLight(lightpos);	//	对灯光采样，并返回对应的pdf

		vec3 L = lightpos.coords - inter.coords;
		float distance2 = L.dot(L);		//	计算距离的平方
		vec3 V = -ray.Dir;				//	ray.dir是从相机看向交点，但是V需要从交点到相机，所以V = -ray.dir
		vec3 N = inter.normal;
		L.normalize();

		Ray inter2light(inter.coords, L);	//	计算从接触点inter出发沿着L(wo)方向的光线是否能与灯光接触
		Intersection t = getIntersection(inter2light);

		//	光源对inter的直接贡献
		vec3 LightDir(0.f);
		//	如果打到了别的物体，norm就不为0，说明直接光照光路被阻挡了
		if (t.happened && (t.coords - lightpos.coords).norm() < 0.01f) {
			//	直接光照计算公式
			//	lightpos.emit * inter.m->eval(V, L, N)这里的两个vec3的乘法是cwiseProduct，不是点乘dot
			vec3 H = (L + V).normalized();
			LightDir = lightpos.emit * inter.m->eval(V, L, N, H)
				* L.dot(N) * lightpos.normal.dot(-L) / (lightpdf * distance2);
			format(LightDir);
		}
		//	俄罗斯轮盘赌，停止递归
		if (getrandom() > RussianRoulette) {
			return LightDir;
		}

		//	环境对inter的间接贡献
		vec3 LightInDir(0.f);
		Material *m = inter.m;
		vec3 H = inter.m->sample(V, N).normalized();						//	采样半程向量H的方向

		vec3 inDirL(0.f);
		float inDirpdf = 0.f;

		if (m->getType() == MICROFACET 
			&& inter.m->isMetal == false && getrandom() > m->fresnel(V,H,m->F0)) {
			//	以1-F的概率折射
			//	折射方向
			inDirL = m->refract(-V, H);
		}
		else {
			//	反射方向
			inDirL = m->reflect(V, H);
		}
		inDirpdf = m->pdf(V, inDirL, N, H);
		if (inDirpdf > 0.f) {
			Ray next(inter.coords, inDirL);
			Intersection inter2 = getIntersection(next);
			if (inter2.happened) {
				LightInDir = castRay(next, depth + 1)
					* m->eval(V, inDirL, N, H) * abs(N.dot(inDirL)) / (inDirpdf * RussianRoulette);
			}
			format(LightInDir);
		}
		//	返回直接光照和间接光照结果
		return LightInDir + LightDir;
	}

	vec3 castRay1(const Ray& ray, unsigned int depth) const {
		// TO DO Implement Path Tracing Algorithm here
		//  eye					light
		//    ↖               ↗ ↓ light.norm
		//   -V ↖           ↗ L
		//        ↖   N   ↗
		//          ↖ ↑ ↗
		//            pos
		//  ray = o + td
		//  if the ray landed on the object,there's a direct contact point : pos
		//  if the pos is bright,return the emission of this pos
		//  else,randomly take a light from light L_i and get its' pdf with function [sampleLight(L_i, pdf)]
		//  L_i is a bright place.not just a light

		//  then distance = (L_i.postion - pos.postion)^2
		//  wo = (L_i.postion-pos.postion).norm()
		//  wi = ray.dir
		//  cos_theta = N * wo
		//  cos_theta_x = -wo * n'
		//  f_r = pos.m->eval(wi,wo,N)
		//  L_dir = L_i.emission * f_r * cos_theta * cos_theta_x / distance / pdf

		//  if the random num > RR,stop caculate the reflect from other place(except light)
		//  caculate L_indir
		//  eye                 pos2
		//    ↖               ↗
		//   -V ↖           ↗indirL
		//        ↖   N   ↗
		//          ↖ ↑ ↗
		//            pos
		//  wi_ = wi
		//  here,use [pos.m->sample(wi_,N)] to get a wo_
		//  the reflectray = {pos,wo_}
		//  caculate pos2 with intersect(reflectray)
		//  the pos2 can't be a light so [pos2.m->hasEmission] should be [false]
		//  the brdf = pos.m->eval(wi_,wo_,N)
		//  cos_theta_ = N * wo_
		//  pdf_ = pos.m.pdf(wi,N)
		//  pos2.emssion = castray(reflectray,depth+1)
		//  L_indir = pos2.emssion * brdf * cos_theta_ / pdf_ / RR

		//	将结果规整，防止出现负数
		auto format = [&](vec3& a) {
			if (a.x < 0) a.x = 0;
			if (a.y < 0) a.y = 0;
			if (a.z < 0) a.z = 0;
		};
		//	如果从相机来的光线没办法和场景接触，说明没看到东西
		Intersection inter = getIntersection(ray);
		if (!inter.happened) {
			return vec3(0.f);
		}

		if (inter.m->hasEmission()) {
			return inter.emit;
		}
		//	如果看到了灯光，但不是直接看到的(depth!=0),就不管,加速收敛的trick,但是会导致没有灯光的反射
		//if (inter.m->hasEmission()) {
		//	return depth == 0 ? inter.emit : vec3(0.f);
		//}

		Intersection lightpos;
		float lightpdf = SampleLight(lightpos);	//	对灯光采样，并返回对应的pdf

		vec3 L = lightpos.coords - inter.coords;
		float distance2 = L.dot(L);		//	计算距离的平方
		vec3 V = -ray.Dir;				//	ray.dir是从相机看向交点，但是V需要从交点到相机，所以V = -ray.dir
		vec3 N = inter.normal;
		L.normalize();

		Ray inter2light(inter.coords, L);	//	计算从接触点inter出发沿着L(wo)方向的光线是否能与灯光接触
		Intersection t = getIntersection(inter2light);

		//	光源对inter的直接贡献
		vec3 LightDir(0.f);
		//	如果打到了别的物体，norm就不为0，说明直接光照光路被阻挡了
		if (t.happened && (t.coords - lightpos.coords).norm() < 0.01f) {
			//	直接光照计算公式
			//	lightpos.emit * inter.m->eval(V, L, N)这里的两个vec3的乘法是cwiseProduct，不是点乘dot
			vec3 H = (L + V).normalized();
			LightDir = lightpos.emit * inter.m->eval(V, L, N, H)
				* L.dot(N) * lightpos.normal.dot(-L) / (lightpdf * distance2);
			format(LightDir);
		}
		//	俄罗斯轮盘赌，停止递归
		if (getrandom() > RussianRoulette) {
			return LightDir;
		}
		//	这个是对的
		//	环境对inter的间接贡献
		vec3 LightInDir(0.f);
		vec3 inDirL(0.f);
		float inDirpdf = 0.f;
		vec3 H = inter.m->sample(V, N);
		vec3 brdf = inter.m->caculateBSDF(V, N, H, inDirL, inDirpdf);
		inDirpdf = inter.m->pdf(V, inDirL, N, H);
		if (inDirpdf > 0.f) {
			Ray next(inter.coords, inDirL);
			Intersection nextinter = getIntersection(next);
			if (nextinter.happened) {
				LightInDir = castRay(next, depth + 1)
					* brdf * abs(N.dot(inDirL)) / (inDirpdf * RussianRoulette);
			}
		}
		format(LightInDir);		
		//	返回直接光照和间接光照结果
		return LightInDir + LightDir;
	}
};