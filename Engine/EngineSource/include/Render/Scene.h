#pragma once
#include <stdint.h>
#include "Math/math.h"

namespace Engine
{
	class Window;
	class Camera;

	class Scene
	{
	public:

		Scene();
		Scene(const sphere& s) : sphr(s), redrawScene(true) {}

		void render(Window& window, Camera& camera);
		uint32_t PerPixel(int x, int y);

		void moveSphere(vec3 direction);
		void setSpherePosition(vec3 position);

		void redraw(bool redraw) { redrawScene = redraw; }
		sphere& getSphere() { return sphr; }
		vec2 getBR() const { return BR; }
		vec2 getTL() const { return TL; }

	private:

		bool redrawScene;
		sphere sphr;
		vec2 TL, BR;

		Camera* s_camera;
	};
}
