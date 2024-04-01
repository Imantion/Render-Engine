#pragma once
#include <stdint.h>
#include <vector>
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

	static void processKeyboardInput(uint32_t keycode, bool wasDown, bool isDown);
	static void processMouseInput(WPARAM wParam, LPARAM lParam);
	static void updateMousePosition(LPARAM lParam);

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

	Engine::vec2 mousePositionRelativeToBuffer();

	static std::vector<KeyState> keyboard;
	static std::vector<KeyState> mouse;
	static Engine::vec2 mousePosition;


	std::shared_ptr<Engine::Window> window;
	std::shared_ptr < Engine::Scene> scene;

};

