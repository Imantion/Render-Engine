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
#include "Graphics/TextureManager.h"
#include "Graphics/TransformSystem.h"
#include "Graphics/ReflectionCapture.h"

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

Engine::vec3 Fibonacci(int i, int N)
{
	const float GOLDEN_RATIO = (1.0 + sqrt(5.0)) / 2.0;
	float theta = 2.0 * M_PI * i / GOLDEN_RATIO;
	float phiCos  = 1.0 - (i + 0.5) / N;
	float phiSin = sqrt(1.0 - phiCos * phiCos);
	float thetaCos = cosf(theta), thetaSin = sinf(theta);
	return Engine::vec3(thetaCos * phiSin, thetaSin * phiSin, phiCos).normalized();

}

void basisFromDir(Engine::vec3& right, Engine::vec3& top, Engine::vec3& dir)
{
	float k = 1.0 / Engine::Max(1.0 + dir.z, 0.00001);
	float a = dir.y * k;
	float b = dir.y * a;
	float c = -dir.x * a;
	right = Engine::vec3(dir.z + b, c, -dir.x);
	top = Engine::vec3(c, 1.0 - b, -dir.y);
}


int main(int argc, char* argv[])
{

	float sum = 0;	
	float N = 4096;
	Engine::vec3 normal = Engine::vec3(0.707f, 0.0f, 0.707f).normalized();
	for (size_t i = 0; i < N; i++)
	{
		Engine::vec3 right, top, direction = Fibonacci(i, N);
		basisFromDir(right, top, normal);
		direction = right * direction.x + top * direction.y + normal * direction.z;
		sum += Engine::dot(direction,normal);
	}	

	std::cout << sum << " " << 2 * M_PI * sum / N;


	MSG msg = { 0 };

	

	Engine::Timer timer;
	
	Engine::Engine::Init();

	auto skyboxTexture = Engine::TextureManager::Init()->LoadFromFile("skybox", L"Textures\\lake_beach.dds");
	skyboxTexture->BindTexture(0u);
	ID3D11RenderTargetView* rtv = nullptr;
	ID3D11Texture2D* tex = nullptr;

	ReflectionCapture::GenerateCubeMap(rtv, tex, 1024, 1024);
	
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