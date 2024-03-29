#include <iostream>
#include <Window/Window.h>
#include "Render/Scene.h"

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
	}
		
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int main()
{
	Engine::Window window(WindowProc);
	Engine::Scene scene;
	MSG msg = { 0 };

	while (!window.isClosed())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
	
		}
	
		scene.render(window);
		window.flush();
	}
	return 0;
}

