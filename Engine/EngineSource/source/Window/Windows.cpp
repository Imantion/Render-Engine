#include <Window/Window.h>
#include <assert.h>
#include <iostream>
#include <stdint.h>
#include <Graphics/D3D.h>
#include <Graphics/Renderer.h>
using namespace Engine;


Window::Window(int wWidth, int wHeight, WinProc WindowProc)
{

	aspectRatio = (float)wWidth / wHeight;
	wasResized = true;

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpszClassName = L"Window";
	wc.lpfnWndProc = WindowProc;	

	RegisterClassEx(&wc);
	RECT rc = { 0, 0, wWidth, wHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW | WS_SIZEBOX, false);

	int w_width = rc.right - rc.left;
	int w_height = rc.bottom - rc.top;

	m_handle = CreateWindowEx(NULL, L"Window", L"RasterRenderer", WS_OVERLAPPEDWINDOW | WS_SIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT,
		w_width, w_height, NULL, nullptr, nullptr, nullptr);

	HWND w_handle = (HWND)m_handle;
	

	assert(w_handle);

	SetWindowLongPtr(w_handle, GWLP_USERDATA, (LONG_PTR)this);

	window_device_context = GetDC(w_handle);

	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (FLOAT)wWidth;
	viewport.Height = (FLOAT)wHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	if (D3D* d3d = D3D::GetInstance())
	{

		DXGI_SWAP_CHAIN_DESC1  sd = {};
		sd.Width = 0;
		sd.Height = 0;
		sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.Scaling = DXGI_SCALING_NONE;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 2;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.Flags = 0;

		HRESULT hr = d3d->GetFactory()->CreateSwapChainForHwnd(d3d->GetDevice(), w_handle, &sd, nullptr, nullptr, &swapchain);
		assert(SUCCEEDED(hr));
	}
	
	ShowWindow(w_handle, SW_SHOW);
	UpdateWindow(w_handle);



	closed = false;

}

Window::~Window()
{
	DestroyWindow((HWND)m_handle);
}

void Window::onDestroy()
{
	closed = true;
	ReleaseDC(HWND(m_handle), window_device_context);
}

void Window::onResize()
{
	RECT rc;
	GetClientRect((HWND)m_handle, &rc);

	width = rc.right - rc.left;
	height = rc.bottom - rc.top;

	aspectRatio = (float)width / height;

	D3D* d3d = D3D::GetInstance();
	if (width != 0 && height != 0 && d3d != nullptr)
	{
		d3d->GetContext()->OMSetRenderTargets(0, 0, 0);
		
		backBuffer.ReleaseAndGetAddressOf();
		Engine::Renderer::GetInstance()->ReleaseRenderTarget();

		HRESULT hr;
		hr = swapchain->ResizeBuffers(0u, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, 0);
		assert(SUCCEEDED(hr));

		hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
		assert(SUCCEEDED(hr));
		D3D11_TEXTURE2D_DESC dsc;
		backBuffer->GetDesc(&dsc);
		auto aboba = dsc.Format;
		Renderer::GetInstance()->InitDepthWithRTV(backBuffer.Get(), (UINT)width, (UINT)height);

		viewport.Width = (FLOAT)width;
		viewport.Height = (FLOAT)height;

		d3d->GetContext()->RSSetViewports(1, &viewport);
	}

	wasResized = true;

}

bool Engine::Window::wasWindowResized()
{
	bool returnWasResized = wasResized;
	wasResized = false;
	return returnWasResized;
}
bool Window::isClosed() const
{
	return closed;
}

void Engine::Window::flush()
{
	swapchain->Present(0u, 0u);
}
