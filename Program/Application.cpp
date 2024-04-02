#include "Application.h"
#include "Render/Scene.h"
#include "Window/Window.h"
#include <windowsx.h>
#include <iostream>

std::vector<Application::KeyState> Application::keyboard(4);
std::vector<Application::KeyState> Application::mouse(3);

Engine::vec2 Application::mousePosition;

Application::Application(int windowSize, int windowHeight, WinProc func)
{
	window =  std::make_shared<Engine::Window>(windowSize, windowHeight, func);
	scene = std::make_shared<Engine::Scene>();
}

Application::~Application()
{
}

Engine::vec2 Application::mousePositionRelativeToBuffer()
{
	uint32_t bufferW = window->getBufferWidth();
	uint32_t windowW = window->getWindowWidth();
	float x = mousePosition.x * ((bufferW < windowW) ? (float)bufferW / windowW : 1);

	uint32_t bufferH = window->getBufferHeight();
	uint32_t windowH = window->getWindowHeight();
	float y = mousePosition.y * ((bufferH < windowH) ? (float)bufferH / windowH : 1);

	return Engine::vec2(x, y);
}

void Application::processKeyboardInput(uint32_t keycode, bool wasDown, bool isDown)
{
	switch (keycode)
	{
	case 'W':
		keyboard[Application::KeyboardButtons::W].isDown = isDown;
		keyboard[Application::KeyboardButtons::W].wasDown = wasDown;
		break;
	case 'A':
		keyboard[Application::KeyboardButtons::A].isDown = isDown;
		keyboard[Application::KeyboardButtons::A].wasDown = wasDown;
		break;
	case 'S':
		keyboard[Application::KeyboardButtons::S].isDown = isDown;
		keyboard[Application::KeyboardButtons::S].wasDown = wasDown;
		break;
	case 'D':
		keyboard[Application::KeyboardButtons::D].isDown = isDown;
		keyboard[Application::KeyboardButtons::D].wasDown = wasDown;
		break;
	default:
		break;
	}
}

void Application::processMouseInput(WPARAM wParam, LPARAM lParam)
{
	for (size_t i = 0; i < mouse.size(); i++)
	{
		Application::MouseButtons button = (Application::MouseButtons)i;
		mouse[button].wasDown = mouse[button].isDown;
		mouse[button].isDown = (((int)button + 1) & wParam) == i + 1; // button starts count from 0. wParam defines RMB,MMB,LMB counting from 1
																	 // That's why adding 1 to resolv this 
	}
}

void Application::updateMousePosition(LPARAM lParam)
{
	mousePosition.x = GET_X_LPARAM(lParam);
	mousePosition.y = GET_Y_LPARAM(lParam);
}

void Application::update(float deltaTime)
{
	scene->render(*window);

	Engine::vec3 sphereMoveDirection = (0, 0, 0);
	if (keyboard[Application::KeyboardButtons::W].isDown)
		sphereMoveDirection += Engine::vec3(0, 1 * deltaTime, 0);
	if (keyboard[Application::KeyboardButtons::A].isDown)
		sphereMoveDirection += Engine::vec3(-1 * deltaTime, 0, 0);
	if (keyboard[Application::KeyboardButtons::S].isDown)
		sphereMoveDirection += Engine::vec3(0, -1 * deltaTime, 0);
	if (keyboard[Application::KeyboardButtons::D].isDown)
		sphereMoveDirection += Engine::vec3(1 * deltaTime, 0, 0);

	if (mouse[Application::MouseButtons::LEFT].isDown)
	{
		Engine::vec2 position = mousePositionRelativeToBuffer();
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
