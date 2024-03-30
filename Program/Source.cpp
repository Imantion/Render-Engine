#include <Window/Window.h>
#include "Render/Scene.h"
#include "Application.h"

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

		bool wasDown = (lParam & (1 << 30)) != 0;
		bool isDown = (lParam & (1 << 31)) == 0;

		Application::processKeyboardInput(VKCode, wasDown, isDown);
	} break;
		
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int main()
{
	Application app(400, 300, WindowProc);

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
		
		app.update(0.01);
	}
	return 0;
}

