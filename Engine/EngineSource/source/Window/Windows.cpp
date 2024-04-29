#include <Window/Window.h>
#include <assert.h>
#include <iostream>
#include <stdint.h>
#include <Graphics/D3D.h>
using namespace Engine;


Window::Window(int wWidth, int wHeight, WindowProcPtr WindowProc)
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

	m_handle = CreateWindowEx(NULL, L"Window", L"RayTracing", WS_OVERLAPPEDWINDOW | WS_SIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT,
		w_width, w_height, NULL, nullptr, nullptr, nullptr);

	HWND w_handle = (HWND)m_handle;

	ResizeFrameBuffer(wWidth, wHeight);
	

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
		Microsoft::WRL::ComPtr<IDXGIFactory2> factory;

		HRESULT hr = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), &factory);
		assert(SUCCEEDED(hr));

		DXGI_SWAP_CHAIN_DESC1  sd = {};
		sd.Width = 0;
		sd.Height = 0;
		sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.Scaling = DXGI_SCALING_STRETCH;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;

		 hr = factory->CreateSwapChainForHwnd(d3d->GetDevice(), w_handle, &sd, nullptr, nullptr, &swapchain);
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

	
	

	if (D3D* d3d = D3D::GetInstance())
	{
		d3d->GetContext()->OMSetRenderTargets(0, 0, 0);

		pRenderTarget.ReleaseAndGetAddressOf();

		HRESULT hr;
		hr = swapchain->ResizeBuffers(0u, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, 0);
		assert(SUCCEEDED(hr));

		Microsoft::WRL::ComPtr<ID3D11Resource> buffer;
		hr = swapchain->GetBuffer(0, __uuidof(ID3D11Resource), &buffer);
		assert(SUCCEEDED(hr));

		hr = d3d->GetDevice()->CreateRenderTargetView(buffer.Get(), nullptr, &pRenderTarget);
		assert(SUCCEEDED(hr));

		d3d->GetContext()->OMSetRenderTargets(1u, pRenderTarget.GetAddressOf(), nullptr);

		viewport.Width = (FLOAT)width;
		viewport.Height = (FLOAT)height;

		d3d->GetContext()->RSSetViewports(1, &viewport);
	}

	

	wasResized = true;

	ResizeFrameBuffer(width, height);
}

void Engine::Window::Resize(int wWidth, int wHeight)
{
	ResizeFrameBuffer(wWidth, wHeight);
	wasResized = true;
}

void Engine::Window::ResizeFrameBuffer(int bWidth, int bHeight)
{
	buffer.memory.release();
	buffer.width = bWidth;
	buffer.height = bHeight;
	buffersize = buffer.width * buffer.height * 4;

	buffer.pitch = buffer.width * 4;
	buffer.memory = get_ptr(buffersize);
	BITMAPINFO bitmap_info;
	bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
	bitmap_info.bmiHeader.biWidth = buffer.width;
	bitmap_info.bmiHeader.biHeight = buffer.height;
	bitmap_info.bmiHeader.biPlanes = 1;
	bitmap_info.bmiHeader.biBitCount = 32;
	bitmap_info.bmiHeader.biCompression = BI_RGB;
	buffer.bitmap_info = bitmap_info;
}

void Window::setPixel(int x, int y)
{
	if (x < 0 || x > buffer.width || y < 0 || y > buffer.height)
		return;

	uint32_t color = 0x0000ff;
	uint8_t* row = (uint8_t*)buffer.memory.get() + x * 4 + y * buffer.pitch;
	uint32_t* pixel = (uint32_t*)row;
	*pixel = color;

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
	/*StretchDIBits(window_device_context, 0, 0, width, height, 0, 0, buffer.width, buffer.height, buffer.memory.get(), &(buffer.bitmap_info), DIB_RGB_COLORS, SRCCOPY);*/
	swapchain->Present(0u, 0u);
}

void Engine::Window::clearScreen()
{
}
