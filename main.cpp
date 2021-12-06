// PathTracing.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//	原作者：Games101助教团队
//	重构by：BestBlade

#include <iostream>
#include <mutex>
#include "Function.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"

using namespace std;

unsigned int width = 800;
unsigned int height = 800;
float fov = 40;
vec3 eyePos(278, 273, -800);
unsigned int SPP = 1024;
mutex g_mutex;

//	声明函数
void RenderSingleThread(const Scene& scene);
void RenderMultiThread(const Scene& scene);

int main() {
	Scene scene(width, height);
	scene.fov = fov;
	scene.RussianRoulette = 0.8f;

	vec3 emitFalse(0.f);
	vec3 emitTrue(
		vec3(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) * 8.0f
		+ vec3(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) * 15.6f
		+ vec3(0.737f + 0.642f, 0.737f + 0.159f, 0.737f) * 18.4f);

	//	创建材质
	Material* redDiffuse = new Material(MaterialType::DIFFUSE, emitFalse);
	redDiffuse->Kd = vec3(0.63f, 0.065f, 0.05f);
	Material* whiteDiffuse = new Material(MaterialType::DIFFUSE, emitFalse);
	whiteDiffuse->Kd = vec3(0.725f, 0.71f, 0.68f);
	Material* greenDiffuse = new Material(MaterialType::DIFFUSE, emitFalse);
	greenDiffuse->Kd = vec3(0.14f, 0.45f, 0.091f);
	Material* gold = new Material(MaterialType::MICROFACET, emitFalse);
	gold->Kd = vec3(1.00f, 0.85f, 0.57f); gold->F0 = 0.95f; gold->roughness = 0.1f;
	Material* sliver = new Material(MaterialType::MICROFACET, emitFalse);
	sliver->Kd = vec3(0.97f, 0.96f, 0.91f); sliver->F0 = 0.999f; sliver->roughness = 0.01f;

	Material* light = new Material(MaterialType::DIFFUSE, emitTrue);
	light->Kd = vec3(0.65f);

	Mesh floor("./cornellbox/floor.obj", whiteDiffuse);
	Mesh shortbox("./cornellbox/shortbox.obj", gold);
	Mesh tallbox("./cornellbox/tallbox.obj", sliver);
	Mesh left("./cornellbox/left.obj", redDiffuse);
	Mesh right("./cornellbox/right.obj", greenDiffuse);
	Mesh light_("./cornellbox/light.obj", light);

	scene.Add(&floor);
	scene.Add(&shortbox);
	scene.Add(&tallbox);
	scene.Add(&left);
	scene.Add(&right);
	scene.Add(&light_);

	scene.buildBVH();
	//	多线程渲染
	RenderMultiThread(scene);
	//	单线程渲染，调试用
	//RenderSingleThread(scene);

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

	int bx = 4, by = 4;
	int nx = (scene.width + bx - 1) / bx, ny = (scene.height + by - 1) / by;
	std::thread th[4 * 4];
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

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
