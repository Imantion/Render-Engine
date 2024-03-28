#include <Window/Window.h>
#include <assert.h>
#include <stdint.h>
using namespace Engine;



Window::Window(WindowProcPtr WindowProc)
{
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpszClassName = L"Window";
	wc.lpfnWndProc = WindowProc;

	RegisterClassEx(&wc);
	RECT rc = { 0, 0, 800, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, false);

	int w_width = rc.right - rc.left;
	int w_height = rc.bottom - rc.top;

	m_handle = CreateWindowEx(NULL, L"Window", L"RayTracing", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 
		w_width, w_height, nullptr, nullptr, nullptr, nullptr);

	GetClientRect((HWND)m_handle, &rc);
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;

	memory = VirtualAlloc(0, width * height * 4, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
	bitmap_info.bmiHeader.biWidth = width;
	bitmap_info.bmiHeader.biHeight = height;
	bitmap_info.bmiHeader.biPlanes = 1;
	bitmap_info.bmiHeader.biBitCount = 32;
	bitmap_info.bmiHeader.biCompression = BI_RGB;

	assert(m_handle);

	SetWindowLongPtr((HWND)m_handle, GWLP_USERDATA, (LONG_PTR)this);

	

	ShowWindow((HWND)m_handle, SW_SHOW);
	UpdateWindow((HWND)m_handle);

	

}

Window::~Window()
{
	DestroyWindow((HWND)m_handle);
}

void Window::onDestroy()
{
	m_handle = nullptr;
}

bool Window::isClosed() const
{
	return !m_handle;
}

void Engine::Window::stretch()
{
	
	StretchDIBits(GetDC((HWND)m_handle), 0, 0, width, height, 0, 0, width, height, memory, &bitmap_info, DIB_RGB_COLORS, SRCCOPY);
}
