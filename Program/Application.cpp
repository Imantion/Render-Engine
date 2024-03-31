#include "Application.h"
#include "Render/Scene.h"
#include "Window/Window.h"
#include <windowsx.h>
std::unordered_map<uint32_t, Application::KeyState> Application::keyboard {
	{ 'W', Application::KeyState()},
	{ 'A', Application::KeyState()},
	{ 'S', Application::KeyState()},
	{ 'D', Application::KeyState()},
};

std::unordered_map<Application::MouseButtons, Application::KeyState> Application::mouse{
	{Application::MouseButtons::LEFT, Application::KeyState()},
	{Application::MouseButtons::MIDLE, Application::KeyState()},
	{Application::MouseButtons::RIGHT, Application::KeyState()},
};

Engine::vec2 Application::mousePosition;

Application::Application(int windowSize, int windowHeight, WinProc func)
{
	window = new Engine::Window(windowSize, windowHeight, func);
	scene = new Engine::Scene();
}

Application::~Application()
{
	delete window;
	delete scene;
}

void Application::processKeyboardInput(uint32_t keycode, bool wasDown, bool isDown)
{
	if (keyboard.find(keycode) != keyboard.end())
	{
		keyboard[keycode].isDown = isDown;
		keyboard[keycode].wasDown = wasDown;
	}
}

void Application::processMouseInput(WPARAM wParam, LPARAM lParam)
{
	for (size_t i = 1; i < mouse.size() + 1; i++)
	{
		Application::MouseButtons button = (Application::MouseButtons)i;
		mouse[button].wasDown = mouse[button].isDown;
		mouse[button].isDown = (button & wParam) == i;
	}
}

void Application::updateMousePosition(LPARAM lParam)
{
	mousePosition.x = GET_X_LPARAM(lParam);
	mousePosition.y = GET_Y_LPARAM(lParam);
}

void Application::update(float deltaTime)
{
	Engine::vec3 sphereMoveDirection = (0, 0, 0);
	if (keyboard['W'].isDown)
		sphereMoveDirection += Engine::vec3(0, 1 * deltaTime, 0);
	if (keyboard['A'].isDown)
		sphereMoveDirection += Engine::vec3(-1 * deltaTime, 0, 0);
	if (keyboard['S'].isDown)
		sphereMoveDirection += Engine::vec3(0, -1 * deltaTime, 0);
	if (keyboard['D'].isDown)
		sphereMoveDirection += Engine::vec3(1 * deltaTime, 0, 0);

	if (mouse[Application::MouseButtons::LEFT].isDown)
		sphereMoveDirection += Engine::vec3(1 * deltaTime, 0, 0);

	scene->moveSphere(sphereMoveDirection);	

	scene->render(*window);
	window->flush();
}

bool Application::isOpen()
{
	return !window->isClosed();
}
