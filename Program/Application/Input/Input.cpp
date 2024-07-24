#include "Input.h"
#include "Math/vec.h"
#include <windowsx.h>

std::array<Input::KeyState, 17> Input::keyboard;
std::array<Input::KeyState, 3> Input::mouse;
int Input::scrolledDistance = 0;

Engine::vec2 Input::mousePosition;

void Input::processKeyboardInput(uint32_t keycode, bool isDown, bool wasKeyDown)
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
	case 'N':
		keyboard[Input::KeyboardButtons::N].isDown = isDown;
		keyboard[Input::KeyboardButtons::N].wasDown = wasKeyDown;
		break;
	case VK_CONTROL:
		keyboard[Input::KeyboardButtons::CTRL].isDown = isDown;
		break;
	case VK_SPACE:
		keyboard[Input::KeyboardButtons::SPACE].isDown = isDown;
		break;
	case VK_SHIFT:
		keyboard[Input::KeyboardButtons::SHIFT].isDown = isDown;
		break;
	case 0x31: // keycodes for 1,2,3,4
		keyboard[Input::KeyboardButtons::ONE].isDown = isDown;
		keyboard[Input::KeyboardButtons::ONE].wasDown = wasKeyDown;
		break;
	case 0x32:
		keyboard[Input::KeyboardButtons::TWO].isDown = isDown;
		keyboard[Input::KeyboardButtons::TWO].wasDown = wasKeyDown;
		break;
	case 0x33:
		keyboard[Input::KeyboardButtons::THREE].isDown = isDown;
		keyboard[Input::KeyboardButtons::THREE].wasDown = wasKeyDown;
		break;
	case 0x34:
		keyboard[Input::KeyboardButtons::FOUR].isDown = isDown;
		keyboard[Input::KeyboardButtons::FOUR].wasDown = wasKeyDown;
	case VK_OEM_PLUS:
		keyboard[Input::KeyboardButtons::PLUS].isDown = isDown;
		keyboard[Input::KeyboardButtons::PLUS].wasDown = wasKeyDown;
		break;
	case VK_OEM_MINUS:
		keyboard[Input::KeyboardButtons::MINUS].isDown = isDown;
		keyboard[Input::KeyboardButtons::MINUS].wasDown = wasKeyDown;
		break;
	case 'F':
		keyboard[Input::KeyboardButtons::F].isDown = isDown;
		keyboard[Input::KeyboardButtons::F].wasDown = wasKeyDown;
		break;
	default:
		break;
	}
}

void Input::processMouseInput(WPARAM wParam, LPARAM lParam)
{
	for (size_t i = 0; i < mouse.size(); i++)
	{
		Input::MouseButtons button = (Input::MouseButtons)i;
		mouse[button].wasDown = mouse[button].isDown;
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
	mousePosition.x = (float)GET_X_LPARAM(lParam);
	mousePosition.y = (float)GET_Y_LPARAM(lParam);
}

void Input::resetMousePressed()
{
	for (size_t i = 0; i < mouse.size(); i++)
	{
		mouse[i].wasDown = true; 
	}
}

bool Input::keyPresseed(KeyboardButtons key)
{
	bool pressed = keyboard[key].isDown && !keyboard[key].wasDown;

	if (!keyboard[key].wasDown)
		keyboard[key].wasDown = true;

	return pressed;
}

bool Input::keyIsDown(KeyboardButtons key)
{
	return keyboard[key].isDown; // key enum value represent location in array
}

bool Input::mouseIsDown(MouseButtons button)
{
	return mouse[button].isDown; // button enum value represent button location in array
}

bool Input::mouseWasPressed(MouseButtons button)
{
	return mouse[button].isDown && !mouse[button].wasDown;
}

Engine::vec2 Input::getMousePosition()
{
	return mousePosition;
}

void Input::resetScroll()
{
	scrolledDistance = 0;
}
