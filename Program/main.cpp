#include <Window/Window.h>
#include "Render/Scene.h"
#include "Application.h"
#include <iostream>
#include "Utils/Timer.h"
#include <thread>

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

		window->flush();
	}break;
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		uint32_t VKCode = wParam;
		WORD keyFlags = HIWORD(lParam);

		bool wasDown = (keyFlags & KF_REPEAT) != 0; 
		bool isDown = (keyFlags & KF_UP) == 0;

		Application::processKeyboardInput(VKCode, wasDown, isDown);
	} break;
	
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		Application::processMouseInput(wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		Application::updateMousePosition(lParam);

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int main(int argc, char* argv[])
{
	Application app(800, 400, WindowProc);
	Engine::Timer timer;

	MSG msg = { 0 };

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
			app.update(timer.DeltaTime());
			std::cout << timer.DeltaTime() << std::endl;
		}

		std::this_thread::yield();
	}
	return 0;
}

