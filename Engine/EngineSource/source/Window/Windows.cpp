#include <Window/Window.h>
#include <assert.h>
#include <iostream>
#include <stdint.h>
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
	GetClientRect(w_handle, &rc);
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;

	ResizeFrameBuffer(wWidth, wHeight);
	

	assert(w_handle);

	SetWindowLongPtr(w_handle, GWLP_USERDATA, (LONG_PTR)this);

	device_context = GetDC(w_handle);
	

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
	ReleaseDC(HWND(m_handle), device_context);
}

void Window::onResize()
{
	RECT rc;
	GetClientRect((HWND)m_handle, &rc);

	width = rc.right - rc.left;
	height = rc.bottom - rc.top;

	aspectRatio = (float)width / height;

	wasResized = true;

	ResizeFrameBuffer(width * 0.8, height * 0.8);
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
	StretchDIBits(device_context, 0, 0, width, height, 0, 0, buffer.width, buffer.height, buffer.memory.get(), &(buffer.bitmap_info), DIB_RGB_COLORS, SRCCOPY);
}

void Engine::Window::clearScreen()
{
}
