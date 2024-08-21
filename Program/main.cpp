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
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"


#define FRAME_RATE 60

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	

	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

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




Engine::vec3 Fibonacci(float& NoV, int i, int N)
{
	const float GOLDEN_RATIO = (1.0f + sqrt(5.0f)) / 2.0f;
	float theta = 2.0f * (float)M_PI * i / GOLDEN_RATIO;
	float phiCos = NoV = 1.0f - (i + 0.5f) / N;
	float phiSin = sqrt(1.0f - phiCos * phiCos);
	float thetaCos = cosf(theta), thetaSin = sinf(theta);
	return Engine::vec3(thetaCos * phiSin, thetaSin * phiSin, phiCos).normalized();

}

void basisFromDir(Engine::vec3& right, Engine::vec3& top, Engine::vec3& dir)
{
	float k = 1.0f / Engine::Max(1.0f + dir.z, 0.00001f);
	float a = dir.y * k;
	float b = dir.y * a;
	float c = -dir.x * a;
	right = Engine::vec3(dir.z + b, c, -dir.x);
	top = Engine::vec3(c, 1.0f - b, -dir.y);
}


int main(int argc, char* argv[])
{

	float sum = 0;
	float cosSum = 0;
	int N = 4096;
	Engine::vec3 normal = Engine::vec3(0.707f, 0.0f, 0.707f).normalized();
	for (int i = 0; i < N; i++)
	{
		float NoV;
		Engine::vec3 right, top, direction = Fibonacci(NoV,i, N);
		cosSum += NoV;
		basisFromDir(right, top, normal);
		direction = right * direction.x + top * direction.y + normal * direction.z;
		sum += Engine::dot(direction,normal);
	}	

	std::cout << 2 * M_PI * cosSum / N << " " << 2 * M_PI * sum / N;


	MSG msg = { 0 };

	Engine::Timer timer;
	
	Engine::Engine::Init();

	//auto skyboxTexture = Engine::TextureManager::Init()->LoadFromFile("IBL", L"Textures\\night_street.dds");
	//Engine::ReflectionCapture::IBLdiffuse(L"Textures\\PreCalculatedIBL\\Nightdiffuse.dds", skyboxTexture, 3600);
	/*Engine::ReflectionCapture::IBLspecularIrradiance(L"Textures\\PreCalculatedIBL\\NightspecIrrad.dds", skyboxTexture, 5000, 10);*/
	//Engine::ReflectionCapture::IBLreflectance(L"Textures\\PreCalculatedIBL\\reflectance.dds", skyboxTexture, 1024, 1024);
	
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