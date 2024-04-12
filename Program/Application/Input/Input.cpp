#include "Input.h"
#include "Math/vec.h"
#include <windowsx.h>

std::array<Input::KeyState, 9> Input::keyboard;
std::array<Input::KeyState, 3> Input::mouse;
int Input::scrolledDistance = 0;

Engine::vec2 Input::mousePosition;

void Input::processKeyboardInput(uint32_t keycode, bool isDown)
{
	switch (keycode)
	{
	case 'W':
		keyboard[Input::KeyboardButtons::W].isDown = isDown;
		break;
	case 'A':
		keyboard[Input::KeyboardButtons::A].isDown = isDown;
		break;
	case 'S':
		keyboard[Input::KeyboardButtons::S].isDown = isDown;
		break;
	case 'D':
		keyboard[Input::KeyboardButtons::D].isDown = isDown;
		break;
	case 'Q':
		keyboard[Input::KeyboardButtons::Q].isDown = isDown;
		break;
	case 'E':
		keyboard[Input::KeyboardButtons::E].isDown = isDown;
		break;
	case VK_CONTROL:
		keyboard[Input::KeyboardButtons::CTRL].isDown = isDown;
		break;
	case VK_SPACE:
		keyboard[Input::KeyboardButtons::SPACE].isDown = isDown;
		break;
	case VK_SHIFT:
		keyboard[Input::KeyboardButtons::SHIFT].isDown = isDown;
	default:
		break;
	}
}

void Input::processMouseInput(WPARAM wParam, LPARAM lParam)
{
	for (size_t i = 0; i < mouse.size(); i++)
	{
		Input::MouseButtons button = (Input::MouseButtons)i;
		mouse[button].isDown = (((int)button + 1) & wParam) == i + 1; // button starts count from 0. wParam defines RMB,MMB,LMB counting from 1
		// That's why adding 1 to resolv this 
	}
}

void Input::proccesMouseScrolling(WPARAM wParam, LPARAM lParam)
{
	scrolledDistance += GET_WHEEL_DELTA_WPARAM(wParam);
}

void Input::updateMousePosition(LPARAM lParam)
{
	mousePosition.x = GET_X_LPARAM(lParam);
	mousePosition.y = GET_Y_LPARAM(lParam);
}

bool Input::keyIsDown(KeyboardButtons key)
{
	return keyboard[key].isDown; // key enum value represent location in array
}

bool Input::mouseIsDown(MouseButtons button)
{
	return mouse[button].isDown; // button enum value represent button location in array
}

Engine::vec2 Input::getMousePosition()
{
	return mousePosition;
}

void Input::resetScroll()
{
	scrolledDistance = 0;
}
