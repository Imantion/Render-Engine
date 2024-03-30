#include <math.h>
#include "Render/Scene.h"
#include "Window/Window.h"


Engine::Scene::Scene():
	sphr(vec3(0, 0, 5), 0.5)
{	
}

void Engine::Scene::render(Engine::Window& window)
{
	uint32_t* memoryBuffer = (uint32_t*)window.getMemoryBuffer();

	float aspect = (float)window.getWindowWidth() / window.getWindowHeight();
	for (size_t y = 0; y < window.getBufferHeight(); y++)
	{
		for (size_t x = 0; x < window.getBufferWidth(); x++)
		{
			vec2 coord{ (float)x / window.getBufferWidth(), (float)y / window.getBufferHeight() };

			coord = coord * 2.0f - 1;
			coord.x *= aspect;

			memoryBuffer[x + y * window.getBufferWidth()] = PerPixel(coord);
		}
	}
}

void Engine::Scene::moveSphere(vec3 direction)
{
	sphr.position += direction;
}

float Engine::Scene::hitSphere(const ray& r, const sphere& s)
{
	vec3 oc = r.origin - s.position;
	float a = dot(r.direction, r.direction);
	float b = 2 * dot(oc, r.direction);
	float c = dot(oc, oc) - s.radius * s.radius;

	float d = b * b - 4 * a * c;

	if (d < 0)
		return -1;

	return (-b - sqrtf(d)) / (2 * a);
}

uint32_t Engine::Scene::PerPixel(vec2 coord)
{
	vec3 rayDirection = vec3(0, 0, 1);
	vec3 rayOrigin = vec3(coord.x, coord.y, 0);
	ray r = ray(rayOrigin, rayDirection);

	float dist = hitSphere(r, sphr);

	if (dist > 0)
		return 0x00000000 | 255;

	return 0x00000000 | 128 | 128 << 8 | 128 << 16;
}
