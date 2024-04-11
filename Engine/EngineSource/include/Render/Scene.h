#pragma once
#include <stdint.h>
#include <vector>
#include <memory>
#include "Math/math.h"
#include "Math/Mesh.h"

namespace Engine
{
	class Window;
	class Camera;
	class Mesh;

	class Scene
	{
	public:

		Scene();

		void render(Window& window, Camera& camera);
		uint32_t PerPixel(int x, int y);

		void moveSphere(vec3 direction);
		void setSpherePosition(vec3 position);

		void redraw(bool redraw) { redrawScene = redraw; }
		vec2 getBR() const { return BR; }
		vec2 getTL() const { return TL; }

	private:

		bool redrawScene;
		vec2 TL, BR;

		struct cube
		{
			cube() : position(0.0f) {}
			cube(const vec3& position) : position(position) {}
			mat4 transformeMatrix;
			mat4 invTransformeMatrix;
			vec3 position;

			static std::unique_ptr<Mesh> cubeMesh;
		};

		std::vector<sphere> spheres;
		std::vector<cube> cubes;
		Camera* s_camera;
	};
}
