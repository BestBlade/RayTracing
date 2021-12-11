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
	//	�������
	void Add(Object* obj) {
		objects.emplace_back(obj);
	}
	//	��ӵƹ⣨û�õ���
	void Add(std::unique_ptr<Light> light) {
		lights.emplace_back(std::move(light));
	}
	//	��������BVH
	void buildBVH() {
		std::cout << "[!]Generating BVH...\n\n";
		BVH = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
	}

	//	��ȡ���������ԣ����ҷ��ز�����pdf,ֻ��light��
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
	//	��������볡������
	Intersection getIntersection(const Ray& ray)const {
		return BVH->getIntersection(ray);
	}


	vec3 castRay(const Ray& ray, unsigned int depth) const{
		// TO DO Implement Path Tracing Algorithm here
		//  eye					light
		//    �I               �J �� light.norm
		//   -V �I           �J L
		//        �I   N   �J
		//          �I �� �J
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
		//    �I               �J
		//   -V �I           �JindirL
		//        �I   N   �J
		//          �I �� �J
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

		//	�������������ֹ���ָ���
		auto format = [&](vec3& a) {
			if (a.x < 0) a.x = 0;
			if (a.y < 0) a.y = 0;
			if (a.z < 0) a.z = 0;
		};
		//	�����������Ĺ���û�취�ͳ����Ӵ���˵��û��������
		Intersection inter = getIntersection(ray);
		if (!inter.happened) {
			return vec3(0.f);
		}

		if (inter.m->hasEmission()) {
			return inter.emit;
		}
		//	��������˵ƹ⣬������ֱ�ӿ�����(depth!=0),�Ͳ���,����������trick,���ǻᵼ��û�еƹ�ķ���
		//if (inter.m->hasEmission()) {
		//	return depth == 0 ? inter.emit : vec3(0.f);
		//}

		Intersection lightpos;
		float lightpdf = SampleLight(lightpos);	//	�Եƹ�����������ض�Ӧ��pdf

		vec3 L = lightpos.coords - inter.coords;
		float distance2 = L.dot(L);		//	��������ƽ��
		vec3 V = -ray.Dir;				//	ray.dir�Ǵ�������򽻵㣬����V��Ҫ�ӽ��㵽���������V = -ray.dir
		vec3 N = inter.normal;
		L.normalize();

		Ray inter2light(inter.coords, L);	//	����ӽӴ���inter��������L(wo)����Ĺ����Ƿ�����ƹ�Ӵ�
		Intersection t = getIntersection(inter2light);

		//	��Դ��inter��ֱ�ӹ���
		vec3 LightDir(0.f);
		//	������˱�����壬norm�Ͳ�Ϊ0��˵��ֱ�ӹ��չ�·���赲��
		if (t.happened && (t.coords - lightpos.coords).norm() < 0.01f) {
			//	ֱ�ӹ��ռ��㹫ʽ
			//	lightpos.emit * inter.m->eval(V, L, N)���������vec3�ĳ˷���cwiseProduct�����ǵ��dot
			vec3 H = (L + V).normalized();
			LightDir = lightpos.emit * inter.m->eval(V, L, N, H)
				* L.dot(N) * lightpos.normal.dot(-L) / (lightpdf * distance2);
			format(LightDir);
		}
		//	����˹���̶ģ�ֹͣ�ݹ�
		if (getrandom() > RussianRoulette) {
			return LightDir;
		}

		//	������inter�ļ�ӹ���
		vec3 LightInDir(0.f);
		Material *m = inter.m;
		vec3 H = inter.m->sample(V, N).normalized();						//	�����������H�ķ���

		vec3 inDirL(0.f);
		float inDirpdf = 0.f;

		if (m->getType() == MICROFACET 
			&& inter.m->isMetal == false && getrandom() > m->fresnel(V,H,m->F0)) {
			//	��1-F�ĸ�������
			//	���䷽��
			inDirL = m->refract(-V, H);
		}
		else {
			//	���䷽��
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
		//	����ֱ�ӹ��պͼ�ӹ��ս��
		return LightInDir + LightDir;
	}

	vec3 castRay1(const Ray& ray, unsigned int depth) const {
		// TO DO Implement Path Tracing Algorithm here
		//  eye					light
		//    �I               �J �� light.norm
		//   -V �I           �J L
		//        �I   N   �J
		//          �I �� �J
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
		//    �I               �J
		//   -V �I           �JindirL
		//        �I   N   �J
		//          �I �� �J
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

		//	�������������ֹ���ָ���
		auto format = [&](vec3& a) {
			if (a.x < 0) a.x = 0;
			if (a.y < 0) a.y = 0;
			if (a.z < 0) a.z = 0;
		};
		//	�����������Ĺ���û�취�ͳ����Ӵ���˵��û��������
		Intersection inter = getIntersection(ray);
		if (!inter.happened) {
			return vec3(0.f);
		}

		if (inter.m->hasEmission()) {
			return inter.emit;
		}
		//	��������˵ƹ⣬������ֱ�ӿ�����(depth!=0),�Ͳ���,����������trick,���ǻᵼ��û�еƹ�ķ���
		//if (inter.m->hasEmission()) {
		//	return depth == 0 ? inter.emit : vec3(0.f);
		//}

		Intersection lightpos;
		float lightpdf = SampleLight(lightpos);	//	�Եƹ�����������ض�Ӧ��pdf

		vec3 L = lightpos.coords - inter.coords;
		float distance2 = L.dot(L);		//	��������ƽ��
		vec3 V = -ray.Dir;				//	ray.dir�Ǵ�������򽻵㣬����V��Ҫ�ӽ��㵽���������V = -ray.dir
		vec3 N = inter.normal;
		L.normalize();

		Ray inter2light(inter.coords, L);	//	����ӽӴ���inter��������L(wo)����Ĺ����Ƿ�����ƹ�Ӵ�
		Intersection t = getIntersection(inter2light);

		//	��Դ��inter��ֱ�ӹ���
		vec3 LightDir(0.f);
		//	������˱�����壬norm�Ͳ�Ϊ0��˵��ֱ�ӹ��չ�·���赲��
		if (t.happened && (t.coords - lightpos.coords).norm() < 0.01f) {
			//	ֱ�ӹ��ռ��㹫ʽ
			//	lightpos.emit * inter.m->eval(V, L, N)���������vec3�ĳ˷���cwiseProduct�����ǵ��dot
			vec3 H = (L + V).normalized();
			LightDir = lightpos.emit * inter.m->eval(V, L, N, H)
				* L.dot(N) * lightpos.normal.dot(-L) / (lightpdf * distance2);
			format(LightDir);
		}
		//	����˹���̶ģ�ֹͣ�ݹ�
		if (getrandom() > RussianRoulette) {
			return LightDir;
		}
		//	����ǶԵ�
		//	������inter�ļ�ӹ���
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
		//	����ֱ�ӹ��պͼ�ӹ��ս��
		return LightInDir + LightDir;
	}
};