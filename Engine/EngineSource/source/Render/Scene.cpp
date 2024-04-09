#include <math.h>
#include "Render/Scene.h"
#include "Window/Window.h"
#include "Render/Camera.h"

Engine::Scene::Scene():
	sphr(vec3(0, 0, 5), 0.5), redrawScene(true)
{	
}

void Engine::Scene::render(Engine::Window& window, Engine::Camera& camera)
{
	uint32_t* memoryBuffer = (uint32_t*)window.getMemoryBuffer();
	s_camera = &camera;



	if (window.wasWindowResized())
	{
		BR = vec2(1.0 / window.getBufferWidth(), 0);
		TL = vec2(0, 1.0 / window.getBufferHeight());
		s_camera->calculateProjectionMatrix(window.getBufferWidth(), window.getBufferHeight());

		redrawScene = true;
	}

	if (redrawScene)
	{

		for (size_t y = 0; y < window.getBufferHeight(); y++)
		{
			for (size_t x = 0; x < window.getBufferWidth(); x++)
			{

				memoryBuffer[x + y * window.getBufferWidth()] = PerPixel(x,y);

				redrawScene = false;
			}
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



uint32_t Engine::Scene::PerPixel(int x, int y)
{
	vec3 rayDirection = s_camera->getRayDirection(BR * x + TL * y);
	vec3 rayOrigin = s_camera->getPosition();
	ray r = ray(rayOrigin, rayDirection);
	hitInfo hInfo;
	
	
	if (hitSphere(sphr,r, 0.0f, FLT_MAX, hInfo))
	{
		vec3 lightDir = { 1,-1, 0.5 };
		/*vec3 color = (hInfo.normal + 1) * 0.5;*/
		float d = dot(-lightDir, hInfo.normal);
		d = d >= 0 ? d : 0.0001;
		d = d >= 1 ? 1 : d;

		return RGB(255 * 0, 255 * 0, 255 * d);
	}

	
	if (hitPlane(vec3(0, -1, 0), r, vec3(0,-5,0)))
		return RGB(255, 153, 0);

	return RGB(128, 128, 128); // Gray color
}
