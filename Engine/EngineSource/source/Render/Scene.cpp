#include <math.h>
#include "Render/Scene.h"
#include "Window/Window.h"
#include "Render/Camera.h"

std::unique_ptr<Engine::Mesh> Engine::Scene::cube::mesh;

Engine::Scene::Scene() :
	redrawScene(true), cubes(2), spheres(2)
{
	cubes[0].position = vec3(0, 0, 2);
	cubes[0].invTransformeMatrix = inverseTransformMatrix(cubes[0].position, cubes[0].position + vec3(0, 0, 1), vec3(0, 1, 0));

	cubes[1].position = vec3(0, 3, 2);
	cubes[1].invTransformeMatrix = inverseTransformMatrix(cubes[1].position, cubes[1].position + vec3(0, 0, 1), vec3(0, 1, 0));


	spheres[0].position = vec3(2, 2, 4);
	spheres[0].radius = 0.5;

	spheres[1].position = vec3(0, 2, 2);
	spheres[1].radius = 0.5;

	cube::mesh.reset(Mesh::UniteCube());
	cubes[0].bvh = std::make_shared<BVH>(cube::mesh.get());
	cubes[1].bvh = cubes[0].bvh;
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
		s_camera->calculateViewMatrix();
		s_camera->calculateRayDirections();

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
	/*if (direction.length_squared() != 0)
	{
		redrawScene = true;
		sphr.position += direction;
	}*/
	
}

void Engine::Scene::setSpherePosition(vec3 position)
{
	/*if (sphr.position != position)
	{
		redrawScene = true;
		sphr.position = position;
	}*/
}



uint32_t Engine::Scene::PerPixel(int x, int y)
{
	vec3 rayDirection = s_camera->getRayDirection(BR * x + TL * y).normalized();
	vec3 rayOrigin = s_camera->getPosition();
	ray r = ray(rayOrigin, rayDirection);
	hitInfo hInfo;
	
	for (sphere s : spheres)
	{
		if (hitSphere(s, r, 0.0f, FLT_MAX, hInfo))
		{
			vec3 lightDir = { 1,-1, 0.5 };
			float d = dot(-lightDir, hInfo.normal);
			d = d >= 0 ? d : 0.0001;
			d = d >= 1 ? 1 : d;

			return RGB(255 * 0, 255 * 0, 255 * d);
		}
	}

	ray cubeR(r);
	const cube* cubeToRender;
	float t_min;
	for (const cube& c : cubes)
	{
		cubeR.origin = vec3(vec4(r.origin, 1) * c.invTransformeMatrix);
		
		if(c.bvh->intersect(cubeR,hInfo))
			for (size_t i = 0; i < cube::mesh->trianglesAmount(); i++)
			{
				if (hitTriangle(c.mesh->getTriangle(i), cubeR))
				{
					return RGB(0, 255, 0);
				}
			}
	}
	
	

	
	if (hitPlane(vec3(0, -1, 0), r, vec3(0,-5,0)))
		return RGB(255, 153, 0);

	return RGB(128, 128, 128); // Gray color
}
