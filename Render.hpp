#pragma once
#include "Scene.hpp"
#include "Task.hpp"

class Renderer {
public:
	unsigned int SPP;
	vec3 eyePos;

	Renderer(const vec3& eyepos = vec3(0.f), unsigned int spp = 16)
		:SPP(spp), eyePos(std::max(unsigned int(1),spp)) {}

	void Render(const Scene& scene) {
		std::vector<vec3> framebuffer(scene.width * scene.height);
		float scale = tan(radians(scene.fov * .5f));
		float aspectRatio = scene.width * 1.f / scene.height;
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
		fopen_s(&fp, "CornellBox.png", "wb");
		(void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
		for (int i = 0; i < scene.height * scene.width; ++i) {
			static unsigned char color[3];
			color[0] = unsigned char(255 * std::pow(std::max(0.f, std::min(1.f, framebuffer[i].x)), 0.6f));
			color[1] = unsigned char(255 * std::pow(std::max(0.f, std::min(1.f, framebuffer[i].y)), 0.6f));
			color[2] = unsigned char(255 * std::pow(std::max(0.f, std::min(1.f, framebuffer[i].z)), 0.6f));
			fwrite(color, 1, 3, fp);
		}
		fclose(fp);
	}
};