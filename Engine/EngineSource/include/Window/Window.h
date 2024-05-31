#pragma once
#include "Windows.h"
#include "memory"
#include <dxgi1_5.h>
#include <d3d11.h>
#include <mutex>
#include <wrl.h>


typedef LRESULT(CALLBACK* WinProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

namespace Engine
{


	class Window
	{
	public:
		Window(int wWidth, int wHeight, WinProc);
		~Window();

		void onDestroy();
		void onResize();
		bool isClosed() const;

		void flush();
		
		float getAspectRation() const { return aspectRatio; }
		int getWindowHeight() const { return height; }
		int getWindowWidth() const { return width; }
		HWND getHWND() { return (HWND)m_handle; }

		bool wasWindowResized();
	private:

		void* m_handle = nullptr;
		HDC window_device_context;
		int height;
		int width;
		bool closed;
		bool wasResized;
		float aspectRatio;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		D3D11_VIEWPORT viewport;

	};

} // namespace Engine