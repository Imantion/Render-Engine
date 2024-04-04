#pragma once
#include <array>
#include <Windows.h>

namespace Engine
{
	struct vec2;
}

class Input
{
public:
	enum MouseButtons;
	enum KeyboardButtons;

	static void processKeyboardInput(uint32_t keycode, bool isDown);
	static void processMouseInput(WPARAM wParam, LPARAM lParam);
	static void updateMousePosition(LPARAM lParam);

	static bool keyIsDown(KeyboardButtons key);
	static bool mouseIsDown(MouseButtons button);
	static Engine::vec2 getMousePosition();

	

	struct KeyState {
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




private:
	static std::array<KeyState, 4> keyboard;
	static std::array<KeyState, 3> mouse;
	static Engine::vec2 mousePosition;
};
