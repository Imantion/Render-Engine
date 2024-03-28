#include <Window/Window.h>
#include <assert.h>
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

	m_handle = CreateWindowEx(NULL, L"Window", L"RayTracing", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 
		rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, nullptr, nullptr);

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
