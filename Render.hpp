#pragma once
#include "Scene.hpp"
#include "Task.hpp"



struct hitPlace {
	float tNear;
	unsigned int index;
	vec2 uv;
	Object* hitObj;
};

class Renderer {
public:
	vec3 eye_pos;
	int SPP;

	Renderer(const vec3& pos = vec3(0.0f),int spp = 1) :eye_pos(pos),SPP(max(1,spp)) {}

	void Render(const Scene& scene) {
		std::vector<vec3> framebuffer(scene.width * scene.height);
		float scale = tan(radians(scene.fov * 0.5));
		float aspect_ratio = (float)scene.width / scene.height;
		int n = 0;

		std::cout << "SPP = " << SPP << "\n";
		for (unsigned int j = 0; j < scene.height; ++j) {
			for (unsigned int i = 0; i < scene.width; ++i) {
				float x = (2 * (i + 0.5) / float(scene.width) - 1) * aspect_ratio * scale;
				float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;

				vec3 dir = normalize(vec3(-x, y, 1));
				for (int k = 0; k < SPP; ++k) {
					framebuffer[n] += scene.castRay(Ray(eye_pos, dir), 0) / float(SPP);
				}
				n++;
			}
			Visualize(scene.height, j);
		}

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
};