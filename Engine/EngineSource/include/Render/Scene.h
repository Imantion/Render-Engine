#pragma once
#include <stdint.h>
#include "Math/math.h"

namespace Engine
{
	class Window;
	struct vec2;
	struct vec3;

	class Scene
	{
	public:

		Scene();
		Scene(const sphere& s) : sphr(s) {}

		void render(Window& window);
		uint32_t PerPixel(vec2 coord);

		void moveSphere(vec3 direction);
		void setSpherePosition(vec3 position);

		float hitSphere(const ray& r, const sphere& s);

		sphere& getSphere() { return sphr; }
		vec2 getBR() { return BR; }
		vec2 getTL() { return TL; }

	private:

		sphere sphr;
		vec2 TL, BR;
	};
}
