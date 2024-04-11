#include <thread>
#include <iostream>
#include "Window/Window.h"
#include "Render/Scene.h"
#include "App/Application.h"
#include "Input/Input.h"
#include "Utils/Timer.h"
#include "Math/matrix.h"

#define FRAME_RATE 60
Engine::vec2 prevMouse{400,200};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
	case WM_DESTROY:
	{
		Engine::Window* window = (Engine::Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		window->onDestroy();
		return 0;
	}
	case WM_SIZE:
	{
		Engine::Window* window = (Engine::Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		window->onResize();

		window->flush();
	}break;
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	{
		WORD keyFlags = HIWORD(lParam);
		if ((keyFlags & KF_REPEAT) == 0)
			Input::processKeyboardInput(wParam, true);
	} break;
	case WM_SYSKEYUP:
	case WM_KEYUP:
	{
		Input::processKeyboardInput(wParam, false);

	} break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		Input::processMouseInput(wParam, lParam);
	case WM_RBUTTONUP:
		Input::processMouseInput(wParam, lParam);
		break;
	case WM_RBUTTONDOWN:
		Input::processMouseInput(wParam, lParam);
		
		break;
	case WM_MOUSEMOVE:
		Input::updateMousePosition(lParam);

	break;
	case WM_SETCURSOR:
		if (LOWORD(lParam) == HTLEFT || LOWORD(lParam) == HTRIGHT ||
			LOWORD(lParam) == HTTOP || LOWORD(lParam) == HTTOPLEFT ||
			LOWORD(lParam) == HTTOPRIGHT || LOWORD(lParam) == HTBOTTOM ||
			LOWORD(lParam) == HTBOTTOMLEFT || LOWORD(lParam) == HTBOTTOMRIGHT) {
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
		}
		else {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}break;
	case WM_MOUSEWHEEL:
		std::cout << GET_WHEEL_DELTA_WPARAM(wParam) << std::endl;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int main(int argc, char* argv[])
{
	Application app(800, 400, WindowProc);
	Engine::Timer timer;

	MSG msg = { 0 };

	Engine::mat4 Result(0.0f);


	while (app.isOpen())
	{
		
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
	
		}

		if (timer.timeElapsed(FRAME_RATE))
		{
			app.update(timer.getDeltatime());
		}

		std::this_thread::yield();
		
	}
	return 0;
}

