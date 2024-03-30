#include "Application.h"
#include "Render/Scene.h"
#include "Window/Window.h"

std::unordered_map<uint32_t, Application::KeyState> Application::keyboard {
	{ 'W', Application::KeyState()},
	{ 'A', Application::KeyState()},
	{ 'S', Application::KeyState()},
	{ 'D', Application::KeyState()},
};

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

	scene->moveSphere(sphereMoveDirection);	

	scene->render(*window);
	window->flush();
}

bool Application::isOpen()
{
	return !window->isClosed();
}
