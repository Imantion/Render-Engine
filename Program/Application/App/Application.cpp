#include "Application.h"
#include "Render/Scene.h"
#include "Window/Window.h"
#include "Input/Input.h"
#include "Math/math.h"
#include "Render/Camera.h"
#include "Math/quaternion.h"
#include <windowsx.h>
#include <iostream>



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

	Engine::vec2 mousePosition = Input::getMousePosition();
	Engine::vec2 delta = (mousePosition - previousMousePosition) * 0.002;
	previousMousePosition = mousePosition;
	float roll = 0.0f;

	Engine::vec3 cameraMoveDirection = (0, 0, 0);
	if (Input::keyIsDown(Input::KeyboardButtons::W))
		cameraMoveDirection += camera->getForward() * deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::A))
		cameraMoveDirection += camera->getRight() * -deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::S))
		cameraMoveDirection += camera->getForward() * -deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::D))
		cameraMoveDirection += camera->getRight() * deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::CTRL))
		cameraMoveDirection += camera->getUp() * -deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::SPACE))
		cameraMoveDirection += camera->getUp() * deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::E))
		roll -= 0.02;
	if (Input::keyIsDown(Input::KeyboardButtons::Q))
		roll += 0.02;

	/*if (Input::mouseIsDown(Input::MouseButtons::LEFT))
	{
		Engine::vec2 position = WindowCoordinatesToBufferCoordinates(Input::getMousePosition());

		position = scene->getBR() * position.x + scene->getTL() * position.y;
		position = position * 2 - 1;

		Engine::ray r(Engine::vec3(0.0f), Engine::vec3(position.x, position.y, 1));
		auto point = r.point_at_parameter(scene->getSphere().position.z);
		scene->setSpherePosition(Engine::vec3(point.x, -point.y, point.z));
	}*/
	camera->moveCamera(cameraMoveDirection);

	bool cameraRotated = false;

	if (Input::mouseIsDown(Input::MouseButtons::RIGHT))
	{
		if (delta.x != 0 || delta.y != 0)
		{
			cameraRotated = true;

			Engine::quaternion rotation = (Engine::quaternion::angleAxis(delta.x, camera->getUp()) * Engine::quaternion::angleAxis(delta.y, camera->getRight())).normalize();
			camera->setForward(Engine::quaternion::rotate(rotation,camera->getForward()));
		}

		if (roll != 0.0f)
		{
			cameraRotated = true;
			Engine::quaternion r = Engine::quaternion::angleAxis(roll, camera->getForward()).normalize();
			Engine::quaternion rotation = r * Engine::quaternion(0, camera->getUp()) * r.conjugate();
			camera->setUp(Engine::vec3(rotation.im));
		}
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
