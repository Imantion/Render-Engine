#pragma once
#include <stdint.h>

namespace Engine
{
	class Window;
	struct vec2;
	struct vec3;
	struct ray;
	struct sphere;

	class Scene
	{
	public:

		void render(Window& window);

		float hitSphere(const ray& r, const sphere& s);

		uint32_t PerPixel(vec2 coord);
	};
}
