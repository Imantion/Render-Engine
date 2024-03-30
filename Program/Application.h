#pragma once
#include <stdint.h>
#include <unordered_map>
#include <Windows.h>

namespace Engine
{
	class Scene;
	class Window;
}

typedef LRESULT(CALLBACK* WinProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class Application
{
public:

	static void processKeyboardInput(uint32_t keycode, bool wasDown, bool isDown);

	struct KeyState {
		bool wasDown;
		bool isDown;
	};

	void update(float deltaTime);
	bool isOpen();

	Application(int windowSize, int windowHeight, WinProc);
	~Application();

private:

	static std::unordered_map<uint32_t, KeyState> keyboard;
	Engine::Window* window;
	Engine::Scene* scene;

};

