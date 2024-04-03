#include "Application.h"
#include "Render/Scene.h"
#include "Window/Window.h"
#include "Input/Input.h"
#include <windowsx.h>
#include <iostream>



Application::Application(int windowSize, int windowHeight, WinProc func)
{
	window =  std::make_shared<Engine::Window>(windowSize, windowHeight, func);
	scene = std::make_shared<Engine::Scene>();
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
	scene->render(*window);

	Engine::vec3 sphereMoveDirection = (0, 0, 0);
	if (Input::keyIsDown(Input::KeyboardButtons::W))
		sphereMoveDirection += Engine::vec3(0, 1 * deltaTime, 0);
	if (Input::keyIsDown(Input::KeyboardButtons::A))
		sphereMoveDirection += Engine::vec3(-1 * deltaTime, 0, 0);
	if (Input::keyIsDown(Input::KeyboardButtons::S))
		sphereMoveDirection += Engine::vec3(0, -1 * deltaTime, 0);
	if (Input::keyIsDown(Input::KeyboardButtons::D))
		sphereMoveDirection += Engine::vec3(1 * deltaTime, 0, 0);

	if (Input::mouseIsDown(Input::MouseButtons::LEFT))
	{
		Engine::vec2 position = WindowCoordinatesToBufferCoordinates(Input::getMousePosition());
		position = scene->getBR() * position.x + scene->getTL() * position.y;
		position = position * 2 - 1;

		position.x *= window->getAspectRation();

		scene->setSpherePosition(Engine::vec3(position.x, -position.y, scene->getSphere().position.z));
	}

	

	scene->moveSphere(sphereMoveDirection);	

	
	window->flush();
}

bool Application::isOpen()
{
	return !window->isClosed();
}
