#include <math.h>
#include "Render/Scene.h"
#include "Window/Window.h"


Engine::Scene::Scene():
	sphr(vec3(0, 0, 5), 0.5), redrawScene(true)
{	
}

void Engine::Scene::render(Engine::Window& window)
{
	uint32_t* memoryBuffer = (uint32_t*)window.getMemoryBuffer();


	vec2 tempBR = vec2(1.0 / window.getBufferWidth(), 0);
	vec2 tempTL = vec2(0, 1.0 / window.getBufferHeight());

	if (tempBR != BR || tempTL != TL)
	{
		BR = tempBR;
		TL = tempTL;
		redrawScene = true;
	}

	if(redrawScene)
		for (size_t y = 0; y < window.getBufferHeight(); y++)
		{
			for (size_t x = 0; x < window.getBufferWidth(); x++)
			{
				vec2 coord = (BR * x) + (TL * y);

				coord = coord * 2.0f - 1; // Converting coordinate to be in range (-1;1) instead of (0;1) 
				coord.x *= window.getAspectRation(); // Multiplying by aspectration to be resolution independent

				memoryBuffer[x + y * window.getBufferWidth()] = PerPixel(coord);

				redrawScene = false;
			}
		}
}



void Engine::Scene::moveSphere(vec3 direction)
{
	if (direction.length_squared() != 0)
	{
		redrawScene = true;
		sphr.position += direction;
	}
	
}

void Engine::Scene::setSpherePosition(vec3 position)
{
	if (sphr.position != position)
	{
		redrawScene = true;
		sphr.position = position;
	}
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
		return RGB(255,0,0); // Blue color.For DIB bitmaps RGB pallet is in reverse order i.e. BGR

	return RGB(128, 128, 128); // Gray color
}
