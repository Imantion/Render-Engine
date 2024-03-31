#pragma once
#include <stdint.h>
#include <unordered_map>
#include <Windows.h>

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
		LEFT = 1,
		MIDLE,
		RIGHT,
	};

	void update(float deltaTime);
	bool isOpen();

	Application(int windowSize, int windowHeight, WinProc);
	~Application();

private:

	static std::unordered_map<uint32_t, KeyState> keyboard;
	static std::unordered_map<MouseButtons, KeyState> mouse;
	static Engine::vec2 mousePosition;


	Engine::Window* window;
	Engine::Scene* scene;

};

