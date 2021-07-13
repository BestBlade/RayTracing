#include "Vector.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include <mutex>

using namespace std;

int picWidth = 800;
int picHeight = 800;
float fov = 40;
vec3 eye_pos(278, 273, -800);
int SPP = 100; 

mutex g_mutex;
int completeNum = 0;

void Render(const Scene& scene);

int main() {
	Scene scene(picWidth, picHeight);
	scene.fov = fov;
	scene.RussianRoulette = 0.8;

	vec3 emit_false(0.0f);
	//vec3 emit_true(100);
	vec3 emit_true(
		vec3(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) * 8.0f
		+ vec3(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) * 15.6f
		+ vec3(0.737f + 0.642f, 0.737f + 0.159f, 0.737f) * 18.4f
	);

	Material* red = new Material(DIFFUSE, emit_false);
	red->Kd = vec3(0.63f, 0.065f, 0.05f);
	Material* green = new Material(DIFFUSE, emit_false);
	green->Kd = vec3(0.14f, 0.45f, 0.091f);
	Material* white = new Material(DIFFUSE, emit_false);
	white->Kd = vec3(0.725f, 0.71f, 0.68f);
	Material* light = new Material(DIFFUSE, emit_true);
	light->Kd = vec3(0.65f);

	Material* mirror = new Material(MICROFACE, emit_false);
	mirror->Kd = vec3(0.95, 0.93, 0.88);
	mirror->roughness = 0.0001;
	mirror->ior = 30;

	Material* gold = new Material(MICROFACE, emit_false);
	gold->Kd = vec3(0.08, 0.08, 0.08);
	gold->roughness = 0.0001;
	gold->ior = 1.5;


	Mesh floor("./cornellbox/floor.obj", white);
	Mesh shortbox("./cornellbox/shortbox.obj", gold);
	Mesh tallbox("./cornellbox/tallbox.obj", mirror);
	Mesh left("./cornellbox/left.obj", red);
	Mesh right("./cornellbox/right.obj", green);
	Mesh light_("./cornellbox/light.obj", light);


	scene.Add(&floor);
	scene.Add(&shortbox);
	scene.Add(&tallbox);
	scene.Add(&left);
	scene.Add(&right);
	scene.Add(&light_);

	scene.buildBVH();

	Render(scene);

	return 0;
}

void Render(const Scene& scene) {
	std::vector<vec3> framebuffer(scene.width * scene.height);
	float scale = tan(radians(scene.fov * 0.5));
	float aspect_ratio = (float)scene.width / scene.height;
	//int n = 0;

	std::cout << "SPP = " << SPP << "\n";

	int process = 0;
	auto deal = [&](int lx, int rx, int ly, int ry) {
		for (unsigned int j = ly; j <= ry; ++j) {
			int n = j * scene.width + lx;
			for (unsigned int i = lx; i <= rx; ++i) {
				float x = (2 * (i + 0.5) / float(scene.width) - 1) * aspect_ratio * scale;
				float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;

				vec3 dir = normalize(vec3(-x, y, 1));
				for (int k = 0; k < SPP; ++k) {
					//std::cout << "castRay" << std::endl;
					framebuffer[n] += scene.castRay(Ray(eye_pos, dir), 0) / float(SPP);
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
	for (int i = 0, id = 0; i < scene.width; i += nx) {
		for (int j = 0; j < scene.height; j += ny) {
			th[id] = thread(deal, i,min(i + nx, scene.width) - 1,
				j,min(j + ny, scene.height) - 1);
			id++;
		}
	}

	for (int i = 0; i < bx * by; i++) th[i].join();
	Visualize(1.0f);


	FILE* fp;
	fopen_s(&fp, "CornellBox.png", "wb");
	(void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
	for (int i = 0; i < scene.height * scene.width; ++i) {
		static unsigned char color[3];
		color[0] = (unsigned char)(255 * std::pow(max(0.0f, min(1.0f, framebuffer[i].x)), 0.6));
		color[1] = (unsigned char)(255 * std::pow(max(0.0f, min(1.0f, framebuffer[i].y)), 0.6));
		color[2] = (unsigned char)(255 * std::pow(max(0.0f, min(1.0f, framebuffer[i].z)), 0.6));
		fwrite(color, 1, 3, fp);
	}
	fclose(fp);
}