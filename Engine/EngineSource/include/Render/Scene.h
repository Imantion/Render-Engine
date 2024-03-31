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
		void moveSphere(vec3 direction);
		void setSpherePosition(vec3 position);
		void setSpherePosition(vec2 position);

		float hitSphere(const ray& r, const sphere& s);


		uint32_t PerPixel(vec2 coord);

	private:

		sphere sphr;
		vec2 TL, BR;
	};
}
