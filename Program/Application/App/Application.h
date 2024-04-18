#pragma once
#include <stdint.h>
#include <array>
#include <Windows.h>
#include <memory>
#include "Math/vec.h"
#include "Utils/IRayDraggable.h"

namespace Engine
{
	class Scene;
	class Window;
	class Camera;
	struct vec2;
}

typedef LRESULT(CALLBACK* WinProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class Application
{
public:

	void update(float deltaTime);
	bool isOpen();

	Application(int windowSize, int windowHeight, WinProc);
	~Application();

	

private:

	Engine::vec2 WindowCoordinatesToBufferCoordinates(Engine::vec2 coordinates);
	std::unique_ptr<Engine::IRayDraggable> draggable;

	Engine::vec2 previousMousePosition;
	std::shared_ptr<Engine::Window> window;
	std::shared_ptr < Engine::Scene> scene;
	std::shared_ptr < Engine::Camera> camera;

};

