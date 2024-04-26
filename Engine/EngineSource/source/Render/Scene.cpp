#include <math.h>
#include "Render/Scene.h"
#include "Window/Window.h"
#include "Render/Camera.h"
#include "Render/Material.h"
#include <algorithm>

#define PI 3.14159265359f

std::unique_ptr<Engine::Mesh> Engine::Scene::cube::mesh;
Engine::vec3 lightDir = Engine::vec3(2.0f, -1.0f, 0.5f).normalized();
Engine::vec3 lightColor(0.5f, 0.7f, 1.0f);

Engine::Scene::Scene() :
	s_camera(nullptr), redrawScene(true), cubes(2), spheres(2), spotLight(vec3(1.0f, 1.0f, 0.0f), vec3(0.0f, 5.5f, 2.0f), vec3(0.0f, -1.0f, 0.0f), PI * 0.25f, 5.0f),
	pointLight(vec3(0.2f, 0.5f, 1.0f), vec3(2.0f, 2.0f, 2.0f), 2.0f), sunlight(lightDir, vec3(1.0f)), infinitePlane(vec3(0, -5, 0), vec3(0, 1, 0), Material(vec3(0.514f, 0.322f, 0.365f)))
{
	// Initialize cube 0
	cubes[0].setPosition(vec3(0.0f, 0.0f, 2.0f));
	cubes[0].material.color = vec3(0.62f, 0.05f, 0.56f);

	// Initialize cube 1
	cubes[1].setPosition(vec3(0.0f, 3.0f, 2.0f));
	cubes[1].material.color = vec3(0.35f, 0.4f, 0.21f); // Different color for cube 1

	// Initialize sphere 0
	spheres[0].position = vec3(2.0f, 2.0f, 4.0f);
	spheres[0].radius = 0.5f;
	spheres[0].material.color = vec3(0.1f, 0.3f, 0.9f); // Color for sphere 0

	// Initialize sphere 1
	spheres[1].position = vec3(0.0f, 2.0f, 2.0f);
	spheres[1].radius = 0.5f;
	spheres[1].material.color = vec3(0.8f, 0.4f, 0.6f); // Different color for sphere 1


	cube::mesh.reset(Mesh::UniteCube());
}

void Engine::Scene::render(Engine::Window& window, Engine::Camera& camera)
{
	uint32_t* memoryBuffer = (uint32_t*)window.getMemoryBuffer();
	s_camera = &camera;


	if (window.wasWindowResized())
	{
		BR = vec2(1.0f / window.getBufferWidth(), 0.0f);
		TL = vec2(0.0f, 1.0f / window.getBufferHeight());
		s_camera->calculateProjectionMatrix(window.getBufferWidth(), window.getBufferHeight());
		s_camera->calculateViewMatrix();
		s_camera->calculateRayDirections();

		verticalIterator.resize(window.getBufferHeight());
		horizontalIterator.resize(window.getBufferWidth());

		for (int i = 0; i < window.getBufferHeight(); i++)
			verticalIterator[i] = (uint32_t)i;

		for (int i = 0; i < window.getBufferWidth(); i++)
			horizontalIterator[i] = (uint32_t)i;

		redrawScene = true;
	}

	if (redrawScene)
	{

#define MT 1

#if MT == 1
		std::for_each(verticalIterator.begin(), verticalIterator.end(), // for each for mt
			[this, &memoryBuffer](uint32_t y)
			{
				std::for_each(horizontalIterator.begin(), horizontalIterator.end(),
				[this, y, &memoryBuffer](uint32_t x)
					{
						memoryBuffer[x + y * horizontalIterator.size()] = PerPixel(x, y);


					}
		);
			}
		);
#else
		for (size_t y = 0; y < window.getBufferHeight(); y++)
		{
			for (size_t x = 0; x < window.getBufferWidth(); x++)
			{

				memoryBuffer[x + y * window.getBufferWidth()] = PerPixel(x, y);

				redrawScene = false;
			}
		}
#endif
		redrawScene = false;
	}

}

bool Engine::Scene::intersectSpheres(const ray& r, hitInfo& hInfo, objectRef& isectObject)
{
	bool isHited = false;

	for(sphere& s : spheres)
	{
		if (hitSphere(s, r, 0.0f, FLT_MAX, hInfo))
		{
			isectObject.pObject = &s;
			isectObject.pObjectType = IntersectedType::sphere;
			isHited = true;
		}
	}
	return isHited;
}

bool Engine::Scene::intersectPrimitive(const ray& r, hitInfo& hInfo, objectRef& isectObject) // calling primitives because in future there may be another shapes. Not Only Cube
{
	if (cubes.empty())
		return false;

	bool isHited = false;
	const Mesh* cubeMesh = cubes[0].getMesh();

	ray cubeR(r);
	for(primitive& p : cubes)
	{
		cubeR.origin = vec3(vec4(r.origin, 1) * p.invTransformeMatrix);
		hitInfo meshIntersectInfo;
		if (cubeMesh->bvh.intersect(cubeR, meshIntersectInfo))
		{
			for (uint8_t j = 0; j < cubeMesh->trianglesAmount(); j++)
			{
				if (hitTriangle(cubeMesh->getTriangle(j), cubeR, hInfo))
				{
					isectObject.pObject = &p;
					isectObject.pObjectType = IntersectedType::primitive;
					hInfo.p = vec4(hInfo.p, 1) * p.transformeMatrix;
					isHited = true;
				}
			}
		}
	}

	return isHited;
}

Engine::Material Engine::Scene::CheckIntersection(const ray& r, hitInfo& hitedObjectInfo, objectRef& isectObject) // finds intersection and returns intersected object info and color
{
	hitedObjectInfo.reset_parameter_t();
	isectObject.reset();
	
	Material mat(vec3(0.5f));
	
	intersectSpheres(r, hitedObjectInfo, isectObject);
	intersectPrimitive(r, hitedObjectInfo, isectObject);

	if (hitPlane(infinitePlane, r, hitedObjectInfo))
	{
		mat = infinitePlane.material;
		isectObject.pObject = &infinitePlane;
		isectObject.pObjectType = IntersectedType::plane;
	}

	switch (isectObject.pObjectType)
	{
	case Engine::IntersectedType::sphere:
		mat = reinterpret_cast<sphere*>(isectObject.pObject)->material;
		break;
	case Engine::IntersectedType::primitive:
		mat = reinterpret_cast<primitive*>(isectObject.pObject)->material;
		break;
	default:
		break;
	}

	return mat;
}


uint32_t Engine::Scene::PerPixel(int x, int y) // for every pixel of screen called, to define it color
{
	vec3 rayDirection = s_camera->getRayDirection(BR * x + TL * y).normalized();
	vec3 rayOrigin = s_camera->getPosition();
	ray r = ray(rayOrigin, rayDirection);

	hitInfo hitedObjectInfo;
	objectRef isectedObject;
	
	Material hitetObjectMaterial = CheckIntersection(r, hitedObjectInfo, isectedObject);
	vec3 pixelColor(0.0f);

	sphere s;
	s.position = spotLight.position;
	s.radius = 0.1f;

	if (hitSphere(s, r, 0.0f, FLT_MAX, hitedObjectInfo)) // spot light sphere
		return RGB(0, 0, 255);

	s.position = pointLight.position;

	if (hitSphere(s, r, 0.0f, FLT_MAX, hitedObjectInfo)) // point light sphere
		return RGB(255, 0, 0);

	


	if (hitedObjectInfo.is_t_finite())
	{
		float ambientLight = 0.05f;

		// sky light
		r.direction = -lightDir;
		r.origin = hitedObjectInfo.p + r.direction * 0.01f;

		
		vec3 viewVector = (s_camera->getPosition() - hitedObjectInfo.p).normalized();
		
		pixelColor = LightContribution(sunlight, hitedObjectInfo, hitetObjectMaterial, viewVector);

		hitInfo potenionalObjectInfo;
		objectRef potenionalIsecteObject;
		CheckIntersection(r, potenionalObjectInfo, potenionalIsecteObject);
		if (potenionalObjectInfo.is_t_finite())
			pixelColor = pixelColor * 0.65f;

		// spot light
		r.direction = (spotLight.position - hitedObjectInfo.p).normalized();
		r.origin = hitedObjectInfo.p + r.direction * 0.001f;

		float distanceSquared = (spotLight.position - hitedObjectInfo.p).length_squared();
		viewVector = (s_camera->getPosition() - hitedObjectInfo.p).normalized();

		vec3 contribution = LightContribution(spotLight, hitedObjectInfo, hitetObjectMaterial, viewVector);

		CheckIntersection(r, potenionalObjectInfo, potenionalIsecteObject);
		if (potenionalObjectInfo.is_t_finite() && potenionalObjectInfo.t * potenionalObjectInfo.t < distanceSquared)
			contribution *= 0.65f;

		pixelColor += contribution;


		// point light
		r.direction = (pointLight.position - hitedObjectInfo.p).normalized();
		r.origin = hitedObjectInfo.p + r.direction * 0.001f;

		distanceSquared = (pointLight.position - hitedObjectInfo.p).length_squared();
		viewVector = (s_camera->getPosition() - hitedObjectInfo.p).normalized();

		contribution = LightContribution(pointLight, hitedObjectInfo, hitetObjectMaterial, viewVector);

		CheckIntersection(r, potenionalObjectInfo, potenionalIsecteObject);
		if (potenionalObjectInfo.is_t_finite() && potenionalObjectInfo.t * potenionalObjectInfo.t < distanceSquared)
			contribution *= 0.65f;

		pixelColor += contribution;


		return ConvertToRGB(pixelColor);
	}

	float a = TL.y * y;
	return  ConvertToRGB(vec3(1.0f) * (1.0f - a) + vec3(0.5f, 0.7f, 1.0f) * a); // Sky color
}
