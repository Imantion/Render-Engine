#include <thread>
#include <iostream>
#include "Window/Window.h"
#include "Input/Input.h"
#include "Utils/Timer.h"
#include "Math/matrix.h"
#include "Graphics/Engine.h"
#include "D3DApp/D3DApplication.h"
#include "Graphics/Model.h"
#include "Graphics/MeshSystem.h"

#define FRAME_RATE 60

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
	}break;
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	{
		WORD keyFlags = HIWORD(lParam);
		if ((keyFlags & KF_REPEAT) == 0)
			Input::processKeyboardInput((uint32_t)wParam, true, (keyFlags & KF_REPEAT) == KF_REPEAT);
	} break;
	case WM_SYSKEYUP:
	case WM_KEYUP:
	{
		WORD keyFlags = HIWORD(lParam);
		Input::processKeyboardInput((uint32_t)wParam, false, (keyFlags & KF_UP) == KF_UP);

	} break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		Input::processMouseInput(wParam, lParam);
		break;
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
		Input::proccesMouseScrolling(wParam, lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int main(int argc, char* argv[])
{

	MSG msg = { 0 };


	Engine::Timer timer;
	
	Engine::Engine::Init();
	D3DApplication app(800, 400, WindowProc);

	
	while (!app.isClosed())
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;

		}

		if (timer.timeElapsed(FRAME_RATE))
		{
			app.Update(timer.getDeltatime());
			Input::resetScroll();
			Input::resetMousePressed();
		}


		std::this_thread::yield();
	}

	Engine::Engine::Deinit();
	return 0;
}