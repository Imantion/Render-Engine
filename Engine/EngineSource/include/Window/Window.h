#pragma once
#include "Windows.h" // Хіба треба інклюдити WinApi, щоб typedef створити, і це сумно

namespace Engine
{
	typedef LRESULT(CALLBACK* WindowProcPtr)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	class Window
	{
	public:
		Window(WindowProcPtr);
		~Window();

		void onDestroy();
		bool isClosed() const;

		void stretch();

		void* get_memory() { return memory; }

		int getWindowHeight() { return height; }
		int getWindowWidth() { return width; }
	private:
		void* m_handle = nullptr;
		void* memory;
		BITMAPINFO bitmap_info;
		int height;
		int width;
		
	};

} // namespace Engine