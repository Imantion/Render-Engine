#include <thread>
#include <iostream>
#include "Window/Window.h"
#include "Render/Scene.h"
#include "App/Application.h"
#include "Input/Input.h"
#include "Utils/Timer.h"
#include "Math/matrix.h"
#include "Graphics/Engine.h"


#define FRAME_RATE 60
#define D3DAPP
ConstantBuffer cb = { {800.0f,400.0f,0.0f,0.0f}, 0.0f };

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

		cb.g_resolution[0] = window->getWindowWidth();
		cb.g_resolution[1] = window->getWindowHeight();
		cb.g_resolution[2] = cb.g_resolution[3] = 1.0f / (cb.g_resolution[0] * cb.g_resolution[1]);

		window->flush();
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

#ifdef D3DAPP

#define TRIANGLE
int main(int argc, char* argv[])
{

	MSG msg = { 0 };


	Engine::Timer timer;
	Engine::Engine::Init();
	Engine::Window win(800, 400, WindowProc);

#ifdef TRIANGLE
	Engine::Engine::PrepareTriangle();
#else 
	Engine::Engine::PrepareCurlesque();
#endif // triangle



	while (!win.isClosed())
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
			cb.g_time += timer.getDeltatime();


			if (Engine::D3D* d3d = Engine::D3D::GetInstance())
			{

				const float color[] = { 0.5f, 0.5f,0.5f,1.0f };

				D3D11_MAPPED_SUBRESOURCE mappedResource;
				d3d->GetContext()->Map(d3d->pConstBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource);

				ConstantBuffer* data = (ConstantBuffer*)mappedResource.pData;

				memcpy(data, &cb, sizeof(cb));

				d3d->GetContext()->Unmap(d3d->pConstBuffer.Get(), 0u);

				d3d->GetContext()->ClearRenderTargetView(win.pRenderTarget.Get(), color);
				d3d->GetContext()->Draw(4u, 0u);
				win.flush();
			}
			Input::resetScroll();
		}


		std::this_thread::yield();
	}

	Engine::Engine::Deinit();
	return 0;
}
#else

int main(int argc, char* argv[])
{
	Application app(800, 400, WindowProc);
	Engine::Timer timer;

	MSG msg = { 0 };

	while (app.isOpen())
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
			app.updateInput(timer.getDeltatime());
			app.update(timer.getDeltatime());
			Input::resetScroll();
			Input::resetMousePressed();
		}

		std::this_thread::yield();
		
	}
	return 0;
}

#endif // D3DPAPP