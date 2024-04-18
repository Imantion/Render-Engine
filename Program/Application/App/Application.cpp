#include "Application.h"
#include "Render/Scene.h"
#include "Window/Window.h"
#include "Input/Input.h"
#include "Math/math.h"
#include "Render/Camera.h"
#include "Math/quaternion.h"
#include <windowsx.h>
#include <iostream>

#define PI 3.141592653590f

float cameraSpeed = 1.0f;
bool holdCursorPosition = true;


Application::Application(int windowSize, int windowHeight, WinProc func)
{
	window =  std::make_shared<Engine::Window>(windowSize, windowHeight, func);
	scene = std::make_shared<Engine::Scene>();
	camera = std::make_shared<Engine::Camera>();

	camera->calculateProjectionMatrix(window->getBufferWidth(), window->getBufferHeight());
	camera->calculateViewMatrix();
	camera->calculateRayDirections();
}

Application::~Application()
{
}

Engine::vec2 Application::WindowCoordinatesToBufferCoordinates(Engine::vec2 coordinates)
{
	uint32_t bufferW = window->getBufferWidth();
	uint32_t windowW = window->getWindowWidth();
	float x = coordinates.x * ((bufferW < windowW) ? (float)bufferW / windowW : 1);

	uint32_t bufferH = window->getBufferHeight();
	uint32_t windowH = window->getWindowHeight();
	float y = coordinates.y * ((bufferH < windowH) ? (float)bufferH / windowH : 1);

	return Engine::vec2(x, y);
}


void Application::update(float deltaTime)
{
	scene->render(*window, *camera);


	if (Input::mouseIsDown(Input::RIGHT) && holdCursorPosition)
	{
		previousMousePosition = Input::getMousePosition();
		holdCursorPosition = false;
	}

	Engine::vec2 mousePosition = Input::getMousePosition();
	Engine::vec2 delta = (mousePosition - previousMousePosition);
	float roll = 0.0f;

	Engine::vec3 cameraMoveDirection = (0, 0, 0);
	if (Input::keyIsDown(Input::KeyboardButtons::W))
		cameraMoveDirection += camera->getForward() * cameraSpeed * deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::A))
		cameraMoveDirection += camera->getRight() * cameraSpeed * -deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::S))
		cameraMoveDirection += camera->getForward() * cameraSpeed * -deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::D))
		cameraMoveDirection += camera->getRight() * cameraSpeed * deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::CTRL))
		cameraMoveDirection += camera->getUp() * cameraSpeed * -deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::SPACE))
		cameraMoveDirection += camera->getUp() * cameraSpeed * deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::SHIFT))
		cameraMoveDirection *= 5;
	if (Input::keyIsDown(Input::KeyboardButtons::E))
		roll -= 0.5f * deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::Q))
		roll += 0.5f * deltaTime;



	int scrolls = Input::scrollAmount();
	if (scrolls > 0)
		cameraSpeed += cameraSpeed * 0.05 * scrolls;
	else if(scrolls < 0)
		cameraSpeed += cameraSpeed * 0.05 * scrolls;


	if (Input::mouseIsDown(Input::MouseButtons::LEFT))
	{
		if (!draggable)
		{
			Engine::ray r;
			r.direction = camera->getRayDirection(scene->getTL() * (window->getBufferHeight() - Input::getMousePosition().y) + scene->getBR() * Input::getMousePosition().x);
			r.origin = camera->getPosition();

			Engine::hitInfo hitedSphere;
			Engine::sphere* sph = scene->intersectSpheres(r, hitedSphere);

			Engine::hitInfo hitedPrim;
			Engine::primitive* prm = scene->intersectPrimitive(r, hitedPrim);

			if (hitedSphere.t <= hitedPrim.t && sph)
			{
				draggable = std::make_unique<Engine::ISphereDragger>(sph, hitedSphere);
			}
			else if (prm)
			{
				draggable = std::make_unique<Engine::IMeshDragger>(prm, hitedPrim);
			}
			
		}

		if (draggable && (delta.x != 0 || delta.y != 0 || cameraMoveDirection != 0.0f))
		{
			Engine::ray r;
			r.direction = camera->getRayDirection(scene->getTL() * (window->getBufferHeight() - Input::getMousePosition().y) + scene->getBR() * Input::getMousePosition().x);
			r.origin = camera->getPosition();

			draggable->drag(r, camera->getForward(), cameraMoveDirection);

			scene->redraw(true);
		}
	}
	else
	{
		draggable.release();
	}
	camera->moveCamera(cameraMoveDirection);

	bool cameraRotated = false;

	if (Input::mouseIsDown(Input::MouseButtons::RIGHT))
	{
		float multiplier = PI * deltaTime;
		Engine::vec2 rotationSpeed((float)delta.x / (float)window->getBufferWidth() * multiplier, (float)delta.y / (float)window->getBufferHeight() * multiplier);


		if (rotationSpeed.x != 0.0f)
		{
			cameraRotated = true;

			Engine::quaternion r = Engine::quaternion::angleAxis(rotationSpeed.x, camera->getUp()).normalize();
			Engine::quaternion rotation = r * Engine::quaternion(0, camera->getForward()) * r.conjugate();
			camera->setForward(Engine::vec3(rotation.im));
			camera->setRight(Engine::cross(camera->getUp(), camera->getForward()));
		}

		if (rotationSpeed.y != 0.0f)
		{
			cameraRotated = true;

			Engine::quaternion r = Engine::quaternion::angleAxis(rotationSpeed.y, camera->getRight()).normalize();
			Engine::quaternion rotation = r * Engine::quaternion(0, camera->getForward()) * r.conjugate();
			camera->setForward(Engine::vec3(rotation.im));
			camera->setUp(Engine::cross(camera->getForward(), camera->getRight()));
			
		}

		if (roll != 0.0f)
		{
			cameraRotated = true;
			Engine::quaternion r = Engine::quaternion::angleAxis(roll, camera->getForward()).normalize();
			Engine::quaternion rotation = r * Engine::quaternion(0, camera->getUp()) * r.conjugate();
			camera->setUp(Engine::vec3(rotation.im));
			camera->setRight(Engine::cross(camera->getUp(), camera->getForward()));
		}
	}
	else
	{
		holdCursorPosition = true;
	}

	if (cameraMoveDirection != Engine::vec3(0.0f) || cameraRotated)
	{
		scene->redraw(true);
		camera->calculateViewMatrix();
		camera->calculateRayDirections();
	}

	
	window->flush();
}

bool Application::isOpen()
{
	return !window->isClosed();
}
