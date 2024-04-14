#include <math.h>
#include "Render/Scene.h"
#include "Window/Window.h"
#include "Render/Camera.h"
#include "Render/Material.h"

std::unique_ptr<Engine::Mesh> Engine::Scene::cube::mesh;
Engine::vec3 lightDir = Engine::vec3(1, -1, 0.5).normalized();

Engine::Scene::Scene() :
	redrawScene(true), cubes(2), spheres(2)
{
	// Initialize cube 0
	cubes[0].position = vec3(0, 0, 2);
	cubes[0].invTransformeMatrix = inverseTransformMatrix(cubes[0].position, cubes[0].position + vec3(0, 0, 1), vec3(0, 1, 0));
	cubes[0].material.color = vec3(0.62, 0.05, 0.56);

	// Initialize cube 1
	cubes[1].position = vec3(0, 3, 2);
	cubes[1].invTransformeMatrix = inverseTransformMatrix(cubes[1].position, cubes[1].position + vec3(0, 0, 1), vec3(0, 1, 0));
	cubes[1].material.color = vec3(0.35, 0.4, 0.21); // Different color for cube 1

	// Initialize sphere 0
	spheres[0].position = vec3(2, 2, 4);
	spheres[0].radius = 0.5;
	spheres[0].material.color = vec3(0.1, 0.3, 0.9); // Color for sphere 0

	// Initialize sphere 1
	spheres[1].position = vec3(0, 2, 2);
	spheres[1].radius = 0.5;
	spheres[1].material.color = vec3(0.8, 0.4, 0.6); // Different color for sphere 1


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

Engine::Material Engine::Scene::CheckIntersection(const ray& r,hitInfo& hitedObjectInfo)
{
	hitInfo hInfo; hInfo.t = FLT_MAX;
	hitedObjectInfo.t = FLT_MAX;
	Material mat(vec3(0.5f));

	for (const sphere& s : spheres)
	{
		if (hitSphere(s, r, 0.0f, FLT_MAX, hInfo) && hInfo.t < hitedObjectInfo.t)
		{
			hitedObjectInfo = hInfo;
			mat = s.material;
		}
	}

	ray cubeR(r);
	const cube* cubeToRender;
	float t_min;
	for (const cube& c : cubes)
	{
		cubeR.origin = vec3(vec4(r.origin, 1) * c.invTransformeMatrix);

		if (c.bvh->intersect(cubeR, hInfo))
			for (size_t i = 0; i < cube::mesh->trianglesAmount(); i++)
			{
				if (hitTriangle(c.mesh->getTriangle(i), cubeR, hInfo) && hInfo.t < hitedObjectInfo.t)
				{
					hitedObjectInfo = hInfo;
					mat = c.material;
				}
			}
	}




	if (hitPlane(vec3(0, 1, 0), r, hInfo, vec3(0, -5, 0)) && hInfo.t < hitedObjectInfo.t)
	{
		hitedObjectInfo = hInfo;
		mat = vec3(0.514f, 0.322f, 0.365f);
	}
		

	return mat;
}


uint32_t Engine::Scene::PerPixel(int x, int y)
{
	vec3 rayDirection = s_camera->getRayDirection(BR * x + TL * y).normalized();
	vec3 rayOrigin = s_camera->getPosition();
	ray r = ray(rayOrigin, rayDirection);

	hitInfo hitedObjectInfo;
	Material hitetObjectMaterial = CheckIntersection(r, hitedObjectInfo);
	vec3 pixelColor = hitetObjectMaterial.color;


	if (hitedObjectInfo.t < FLT_MAX)
	{
		r.origin = hitedObjectInfo.p + lightDir.p * 0.01;
		r.direction = -lightDir;


		hitInfo potenionalObject;
		CheckIntersection(r, potenionalObject);
		if (potenionalObject.t < FLT_MAX)
			pixelColor *= 0.8f;

		float ambientLight = 0.1f;

		float d = max(dot(-lightDir, hitedObjectInfo.normal),0.001f);

		vec3 viewVector = (s_camera->getPosition() - hitedObjectInfo.p).normalized();
		vec3 halfwayVector = (viewVector - lightDir).normalized();
		float specAmount = pow(max(dot(halfwayVector, hitedObjectInfo.normal), 0.001), 32);
		specAmount *= 0.5f;

		pixelColor = pixelColor * (d + specAmount + ambientLight);

		return ConvertToRGB(pixelColor);
	}

	float a = TL.y * y;
	return  ConvertToRGB(vec3(1.0f) * (1.0f - a) + vec3(0.5, 0.7, 1.0) * a); // Sky color
}
