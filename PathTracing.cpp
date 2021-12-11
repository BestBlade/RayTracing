// PathTracing.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//	原作者：Games101助教团队
//	重构by：BestBlade

#include <iostream>
#include <mutex>
#include "Function.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"
#include "Sphere.hpp"
#include <windows.h>
#undef min
#undef max

using namespace std;

unsigned int width = 1024;
unsigned int height = 1024;
float fov = 40;
vec3 eyePos(278, 273, -800);
unsigned int SPP = 2048;
int threadNum = 256;
mutex g_mutex;

//	声明函数
void RenderSingleThread(const Scene& scene);
void RenderMultiThread(const Scene& scene); 
void RenderExtremeThread(const Scene& scene);

int main() {
	Scene scene(width, height);
	scene.fov = fov;
	scene.RussianRoulette = 0.9f;

	vec3 emitFalse(0.f);
	vec3 emitTrue(
		vec3(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) * 8.0f
		+ vec3(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) * 15.6f
		+ vec3(0.737f + 0.642f, 0.737f + 0.159f, 0.737f) * 18.4f);
	vec3 emit66ccff(vec3(0.4f, 0.8f, 1.f));

	//	创建材质
	Material* redDiffuse = new Material(MaterialType::DIFFUSE, emitFalse);
	redDiffuse->baseColor = vec3(0.63f, 0.065f, 0.05f);
	Material* whiteDiffuse = new Material(MaterialType::DIFFUSE, emitFalse);
	whiteDiffuse->baseColor = vec3(0.725f, 0.71f, 0.68f);
	Material* greenDiffuse = new Material(MaterialType::DIFFUSE, emitFalse);
	greenDiffuse->baseColor = vec3(0.14f, 0.45f, 0.091f);
	Material* gold = new Material(MaterialType::MICROFACET, emitFalse);
	gold->baseColor = vec3(1.00f, 0.85f, 0.57f); gold->F0 = 0.95f; gold->roughness = 0.5f;
	Material* sliver = new Material(MaterialType::MICROFACET, emitFalse);
	sliver->baseColor = vec3(0.97f, 0.96f, 0.91f); sliver->F0 = 0.999f; sliver->roughness = 0.01f;
	Material* aluminum = new Material(MaterialType::MICROFACET, emitFalse);
	aluminum->baseColor = vec3(0.8f, 0.85f, 0.88f); aluminum->F0 = 0.999f; aluminum->roughness = 0.01f;

	Material* glass1 = new Material(MaterialType::MICROFACET, emitFalse, false, 2.5f);
	glass1->baseColor = vec3(0.97f, 0.96f, 0.91f); glass1->roughness = 0.01f;
	Material* glass2 = new Material(MaterialType::MICROFACET, emitFalse, false, 1.5f);
	glass2->baseColor = vec3(emit66ccff * 1.8f); glass2->roughness = 0.5f;

	Material* light = new Material(MaterialType::DIFFUSE, emitTrue);
	light->baseColor = vec3(0.65f);

	Mesh floor("./cornellbox/floor.obj", whiteDiffuse);
	Mesh shortbox("./cornellbox/shortbox.obj", gold);
	Mesh tallbox("./cornellbox/tallbox.obj", aluminum);
	Mesh left("./cornellbox/left.obj", redDiffuse);
	Mesh right("./cornellbox/right.obj", greenDiffuse);
	Mesh light_("./cornellbox/light.obj", light);

	////	测试mysqrt()函数用场景
	////	注意观察两个球的大小（是否相切），和镜面反射的球背纹路
	////	使用此部分时注视点Line80-Line83，Line86-Line87
	//Sphere sp1(vec3(250, 80, 180), 80, glass1);
	//Sphere sp2(vec3(250, 240, 180), 80, glass2);
	//scene.Add(&tallbox);
	//scene.Add(&sp1);
	//scene.Add(&sp2);

	Sphere sp1(vec3(230, 240, 180), 75, glass1);
	Sphere sp2(vec3(350, 55, 100), 55, glass2);
	scene.Add(&sp1);
	scene.Add(&sp2);

	scene.Add(&floor);
	scene.Add(&shortbox);
	scene.Add(&tallbox);
	scene.Add(&left);
	scene.Add(&right);
	scene.Add(&light_);

	scene.buildBVH();

	time_t tStart, tEnd;
	time(&tStart);
	//	多线程渲染
	RenderMultiThread(scene);
	//	单线程渲染，调试用
	//RenderSingleThread(scene);

	time(&tEnd);
	float diff = difftime(tEnd, tStart);
	int hour = diff / 3600;
	int min = diff / 60 - hour * 60;
	int sec = diff - hour * 3600 - min * 60;

	std::cout << "\r BVH Tree Generation Complete !\nTime cost : "
		<< hour << " hours " << min << " min " << sec << " sec \n\n";

	return 0;
}

void RenderMultiThread(const Scene& scene) {
	std::vector<vec3> framebuffer(scene.width * scene.height);
	float scale = tan(radians(scene.fov * 0.5f));
	float aspect_ratio = scene.width * 1.f / scene.height;

	std::cout << "SPP = " << SPP << "\n";

	int process = 0;
	auto deal = [&](unsigned int lx, unsigned int rx, unsigned int ly, unsigned int ry) {
		for (unsigned int j = ly; j <= ry; ++j) {
			int n = j * scene.width + lx;
			for (unsigned int i = lx; i <= rx; ++i) {
				float x = (2 * (i + 0.5f) / (scene.width * 1.f) - 1) * aspect_ratio * scale;
				float y = (1 - 2 * (j + 0.5f) / (scene.height * 1.f)) * scale;

				vec3 dir = vec3(-x, y, 1).normalized();
				Ray eye2inter(eyePos, dir);
				for (unsigned int k = 0; k < SPP; ++k) {
					framebuffer[n] += scene.castRay(eye2inter, 0) / (SPP * 1.f);
				}
				n++;
				process++;
			}
			std::lock_guard<std::mutex> g1(g_mutex);
			Visualize(1.0 * process / scene.width / scene.height);
		}
	};
	int minx = 0, maxx = scene.width - 1;
	int miny = 0, maxy = scene.height - 1;

	int bx = 10, by = 10;
	int nx = (scene.width + bx - 1) / bx, ny = (scene.height + by - 1) / by;
	std::thread th[10 * 10];
	for (unsigned int i = 0, id = 0; i < scene.width; i += nx) {
		for (unsigned int j = 0; j < scene.height; j += ny) {
			th[id] = thread(deal, i, min(i + nx, scene.width) - 1,
				j, min(j + ny, scene.height) - 1);
			id++;
		}
	}

	for (int i = 0; i < bx * by; i++) th[i].join();
	Visualize(1.0f);


	FILE* fp;
	fopen_s(&fp, "CornellBox.ppm", "wb");
	(void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
	for (unsigned int i = 0; i < scene.height * scene.width; ++i) {
		static unsigned char color[3];
		color[0] = (unsigned char)(255 * std::pow(std::max(0.f, std::min(1.f, framebuffer[i].x)), 0.6f));
		color[1] = (unsigned char)(255 * std::pow(std::max(0.f, std::min(1.f, framebuffer[i].y)), 0.6f));
		color[2] = (unsigned char)(255 * std::pow(std::max(0.f, std::min(1.f, framebuffer[i].z)), 0.6f));
		fwrite(color, 1, 3, fp);
	}
	fclose(fp);
}

void RenderSingleThread(const Scene& scene) {
	std::vector<vec3> framebuffer(scene.width * scene.height);
	float scale = tan(radians(scene.fov * .5f));
	float aspectRatio = scene.width * 1. / scene.height;
	int n = 0;
	std::cout << "[!]SPP = " << SPP << "\n\n";

	for (unsigned int j = 0; j < scene.height; ++j) {
		for (unsigned int i = 0; i < scene.width; ++i) {
			float x = (2 * (i + .5f) / scene.width - 1) * aspectRatio * scale;
			float y = (1 - 2 * (j + .5f) / scene.height) * scale;

			vec3 dir = vec3(-x, y, 1).normalized();
			Ray eye2inter(eyePos, dir);
			for (int k = 0; k < SPP; ++k) {
				framebuffer[n] += scene.castRay(eye2inter, 0) / (SPP * 1.f);
			}
			++n;
		}
		Visualize(scene.height, j);
	}

	FILE* fp;
	fopen_s(&fp, "CornellBox.ppm", "wb");
	(void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
	for (int i = 0; i < scene.height * scene.width; ++i) {
		static unsigned char color[3];
		color[0] = unsigned char(255 * std::pow(max(0.f, min(1.f, framebuffer[i].x)), 0.6f));
		color[1] = unsigned char(255 * std::pow(max(0.f, min(1.f, framebuffer[i].y)), 0.6f));
		color[2] = unsigned char(255 * std::pow(max(0.f, min(1.f, framebuffer[i].z)), 0.6f));
		fwrite(color, 1, 3, fp);
	}
	fclose(fp);
}

std::atomic<int> proc(0);
void worker(int j, float si, float scale, const Scene& scene, std::vector<vec3>& fbuffer) {
	int mc = std::ceil(scene.height * 1. / threadNum);
	int start = j * mc;

	for (int k = start; k < start + mc && k < scene.height; ++k) {

		for (uint32_t i = 0; i < scene.width; ++i) {
			// generate primary ray direction
			float x = (2 * (i + 0.5f) / scene.width - 1) * si;
			float y = (1 - 2 * (k + 0.5f) / scene.height) * scale;
			int  idx = k * scene.width + i;
			vec3 dir = vec3(-x, y, 1).normalized();
			for (int q = 0; q < SPP; q++) {
				fbuffer[idx] += scene.castRay(Ray(eyePos, dir), 0) / SPP;
			}
		}
		proc++;
	}

}
void RenderExtremeThread(const Scene& scene) {
	std::vector<vec3> framebuffer(scene.width * scene.height);
	float scale = tan(radians(scene.fov * .5f));
	float aspectRatio = scene.width * 1. / scene.height;
	float si = aspectRatio * scale;
	int n = 0;
	std::cout << "[!]SPP = " << SPP << "\n\n";
	for (uint32_t j = 0; j < threadNum; ++j) {
		std::thread t(worker, j, si, scale, std::ref(scene), std::ref(framebuffer));
		t.detach();
	}

	while (proc < scene.height - 1) {
		Visualize(proc / (float)scene.height);
		Sleep(5);
	}

	std::cout << "Thread fun  run proc=:" << proc << std::endl;
	FILE* fp;
	fopen_s(&fp, "CornellBox.ppm", "wb");
	(void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
	for (auto i = 0; i < scene.height * scene.width; ++i) {
		static unsigned char color[3];
		color[0] = unsigned char(255 * std::pow(max(0.f, min(1.f, framebuffer[i].x)), 0.6f));
		color[1] = unsigned char(255 * std::pow(max(0.f, min(1.f, framebuffer[i].y)), 0.6f));
		color[2] = unsigned char(255 * std::pow(max(0.f, min(1.f, framebuffer[i].z)), 0.6f));
		fwrite(color, 1, 3, fp);
	}
	fclose(fp);
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
