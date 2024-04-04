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




	if (window.wasWindowResized())
	{
		BR = vec2(1.0 / window.getBufferWidth(), 0);;
		TL = vec2(0, 1.0 / window.getBufferHeight());;
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



uint32_t Engine::Scene::PerPixel(vec2 coord)
{
	vec3 rayDirection = vec3(0, 0, 1);
	vec3 rayOrigin = vec3(coord.x, coord.y, 0);
	ray r = ray(rayOrigin, rayDirection);

	hitInfo hInfo;

	if (hitSphere(sphr,r, 0.0f, FLT_MAX, hInfo))
	{
		vec3 color = (hInfo.normal + 1) * 0.5;

		return RGB(255 * color.b, 255 * color.g, 255 * color.r);
	}

	return RGB(128, 128, 128); // Gray color
}
