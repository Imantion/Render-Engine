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

	static void processKeyboardInput(uint32_t keycode, bool isDown, bool wasKeyDown);
	static void processMouseInput(WPARAM wParam, LPARAM lParam);
	static void proccesMouseScrolling(WPARAM wParam, LPARAM lParam);
	static void updateMousePosition(LPARAM lParam);

	static void resetMousePressed();
	static bool keyPresseed(KeyboardButtons key);
	static bool keyIsDown(KeyboardButtons key);
	static bool mouseIsDown(MouseButtons button);
	static bool mouseWasPressed(MouseButtons button);
	static Engine::vec2 getMousePosition();
	static int scrollAmount() { return scrolledDistance / 120; }
	static void resetScroll();

	

	struct KeyState {
		bool isDown;
		bool wasDown;
	};

	enum MouseButtons
	{
		LEFT,
		RIGHT,
		MIDLE,
	};

	enum KeyboardButtons
	{
		W,
		ONE,
		TWO,
		THREE,
		FOUR,
		A,
		S,
		D,
		CTRL,
		SPACE,
		Q,
		E,
		SHIFT,
		N,
		M,
		PLUS,
		MINUS,
		F,
		G,
		DEL,
	};




private:
	static std::array<KeyState, 20> keyboard;
	static std::array<KeyState, 3> mouse;
	static int scrolledDistance;
	static Engine::vec2 mousePosition;
};

