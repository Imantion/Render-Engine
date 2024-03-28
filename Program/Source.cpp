#include <iostream>
#include <Window/Window.h>
#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
	case WM_DESTROY:
		Engine::Window* window = (Engine::Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		window->onDestroy();
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int main()
{
	Engine::Window window(WindowProc);

	MSG msg = { 0 };
	uint32_t* pixels = (uint32_t*)window.get_memory();

	while (!window.isClosed())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;

			uint32_t* pixel = pixels;
			for (size_t i = 0; i < window.getWindowHeight(); i++)
			{
				for (size_t j = 0; j < window.getWindowWidth(); j++)
				{
					if (i * i + j * j <= 100000)
						*pixel = 0xFF0000;
					pixel++;
				}
			}
			
		}
		
		window.stretch();
	}
	return 0;
}

