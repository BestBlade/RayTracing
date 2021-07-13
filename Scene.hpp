#pragma once
#include "Function.hpp"
#include "BVHTree.hpp"
#include "AreaLight.hpp"
#include "Light.hpp"

class Scene {
public:
	int width;
	int height;
	float fov;
	float RussianRoulette;
	BVHAccel* BVH;

	std::vector<Object*> objects;
	std::vector<std::unique_ptr<Light>> lights;

	Scene(int w, int h) :width(w), height(h) {}

	void Add(Object* obj) {
		objects.emplace_back(obj);
	}

	void Add(std::unique_ptr<Light> light) {
		lights.emplace_back(std::move(light));
	}

	void buildBVH() {
		std::cout << " - Generating BVH...\n\n";
		BVH = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
	}

	void sampleLight(Intersection& pos, float& pdf) const {
		float emit_area_sum = 0;
		for (unsigned int i = 0; i < objects.size(); ++i) {
			if (objects[i]->hasEmit()) {
				emit_area_sum += objects[i]->getArea();
			}
		}

		float p = get_random_float() * emit_area_sum;
		emit_area_sum = 0;
		for (int i = 0; i < objects.size(); ++i) {
			if (objects[i]->hasEmit()) {
				emit_area_sum += objects[i]->getArea();
				if (p < emit_area_sum) {
					objects[i]->Sample(pos, pdf);
					break;
				}
			}
		}
	}

	Intersection intersect(const Ray& ray) const {
		return BVH->Intersect(ray);
	}

	bool trace(const Ray& ray, const std::vector<Object*>& obj, float& tNear, unsigned int& index, Object** hitObj) {
		*hitObj = nullptr;
		for (unsigned int i = 0; i < obj.size(); ++i) {
			float tNearI = INT_MAX;
			unsigned int indexI;
			vec2 uvI;
			if (obj[i]->intersect(ray, tNearI, indexI) && tNearI < tNear) {
				tNear = tNearI;
				index = indexI;
				*hitObj = obj[i];
			}
		}
		return (*hitObj != nullptr);
	}

	vec3 castRay(const Ray& ray, int depth) const {
		// TO DO Implement Path Tracing Algorithm here
		//  eye					light
		//    ¨K               ¨J ¡ý light.norm
		//   -wo¨K           ¨Jws
		//        ¨K   N   ¨J
		//          ¨K ¡ü ¨J
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
		//    ¨K               ¨J
		//   -wo¨K           ¨Jwi
		//        ¨K   N   ¨J
		//          ¨K ¡ü ¨J
		//            pos
		//  wi_ = wi
		//  here,use [pos.m->sample(wi_,N)] to get a wo_
		//  the reflectray = {pos,wo_}
		//  caculate pos2 with intersect(reflectray)
		//  the pos2 can't be a light so [pos2.m->hasEmission] should be [false]
		//  the f_r_ = pos.m->eval(wi_,wo_,N)
		//  cos_theta_ = N * wo_
		//  pdf_ = pos.m.pdf(wi,N)
		//  pos2.emssion = castray(reflectray,depth+1)
		//  L_indir = pos2.emssion * f_r_ * cos_theta_ / pdf_ / RR

		auto format = [](vec3& a) {
			if (a.x < 0) a.x = 0;
			if (a.y < 0) a.y = 0;
			if (a.z < 0) a.z = 0;
		};
		Intersection pos = intersect(ray);
		if (!pos.happened) {
			return vec3(0.0f);
		}
		if (pos.m->hasEmission()) {
			if (depth == 0) {
				return pos.emit;
			}
			return vec3(0);
		}

		Intersection light;
		float pdf;
		sampleLight(light, pdf);

		vec3 pos2light = light.coords - pos.coords;
		float d2 = dot(pos2light, pos2light);
		//	w sample
		vec3 ws = normalize(pos2light);
		vec3 wo = normalize(-ray.dir);
		vec3 N = pos.normal;

		Ray pos2lightray(pos.coords, ws);
		Intersection t = intersect(pos2lightray);

		vec3 L_dir(0.0);

		if (t.happened && norm(t.coords - light.coords) < 0.01) {

			L_dir = light.emit * pos.m->eval(ws, wo, N) * dot(ws, N) * dot(-ws, light.normal) / (pdf * d2);

			format(L_dir);
		}
		if (get_random_float() > RussianRoulette) {
			return L_dir;
		}
		//return L_dir;

		vec3 L_indir;

		if (pos.m->getType() == MICROFACE) {
			float pdf_;
			vec3 wi;
			vec3 brdf = pos.m->ggxSample(wo, N, wi, pdf_);
			if (pdf_ > 0) {
				wi = normalize(wi);
				Ray ref(pos.coords, wi);
				Intersection pos2 = intersect(ref);
				if (pos2.happened && !pos2.m->hasEmission()) {
					L_indir = castRay(ref, depth + 1)
						* brdf * fabs(dot(wi, N))
						/ (pdf_ * RussianRoulette);
					format(L_indir);
				}
			}
		}
		else {
			vec3 wi = normalize(pos.m->sample(wo, N));
			L_indir = castRay(Ray(pos.coords, wi), depth + 1)
				* pos.m->eval(wo, wi, N) * dot(wi, N)
				/ (pos.m->pdf(wo, wi, N) * RussianRoulette);
			format(L_indir);
		}
		return L_indir + L_dir;
	}
};
