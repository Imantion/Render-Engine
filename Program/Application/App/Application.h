#pragma once
#include <stdint.h>
#include <array>
#include <Windows.h>
#include <memory>

namespace Engine
{
	class Scene;
	class Window;
	struct vec2;
}

typedef LRESULT(CALLBACK* WinProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class Application
{
public:

	struct KeyState {
		bool wasDown;
		bool isDown;
	};

	enum MouseButtons
	{
		LEFT,
		MIDLE,
		RIGHT,
	};

	enum KeyboardButtons
	{
		W,
		A,
		S,
		D
	};

	void update(float deltaTime);
	bool isOpen();

	Application(int windowSize, int windowHeight, WinProc);
	~Application();

private:

	Engine::vec2 WindowCoordinatesToBufferCoordinates(Engine::vec2 coordinates);


	std::shared_ptr<Engine::Window> window;
	std::shared_ptr < Engine::Scene> scene;

};

