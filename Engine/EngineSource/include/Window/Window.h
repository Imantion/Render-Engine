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
		void onResize();
		void ResizeFrameBuffer(int wWidth, int wHeight);
		bool isClosed() const;

		void flush();
		void clearScreen();
		void setPixel(int x, int y);
		
		void* getMemoryBuffer() { return buffer.memory; }
		int getWindowHeight() { return height; }
		int getWindowWidth() { return width; }
		int getBufferHeight() { return buffer.height; }
		int getBufferWidth() { return buffer.width; }
	private:

		void* m_handle = nullptr;
		HDC device_context;
		int height;
		int width;
		int buffersize;
		bool closed;
		float aspectRatio = 16 / 9;

		struct BitmapBuffer
		{
			int width, height;
			BITMAPINFO bitmap_info;
			void* memory;
			int pitch; // in bytes
		} buffer;
	};

} // namespace Engine