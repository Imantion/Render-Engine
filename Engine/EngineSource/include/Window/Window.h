#pragma once
#include "Windows.h" // Хіба треба інклюдити WinApi, щоб typedef створити, і це сумно
#include "memory"

namespace Engine
{
	typedef LRESULT(CALLBACK* WindowProcPtr)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	class Window
	{
	public:
		Window(int wWidth, int wHeight, WindowProcPtr);
		~Window();

		void onDestroy();
		void onResize();
		void ResizeFrameBuffer(int wWidth, int wHeight);
		bool isClosed() const;

		void flush();
		void clearScreen();
		void setPixel(int x, int y);
		
		float getAspectRation() { return aspectRatio; }
		void* getMemoryBuffer() { return buffer.memory.get(); }
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
		float aspectRatio;

		struct BitmapBuffer
		{
			BitmapBuffer() : memory{0, std::free} {}
			int width, height;
			BITMAPINFO bitmap_info;
			std::unique_ptr<void, decltype(&std::free)> memory;
			int pitch; // in bytes
		} buffer;

		auto get_ptr(std::size_t size) {
			return std::unique_ptr<void, decltype(&std::free)>(std::malloc(size), std::free);
		}
	};

} // namespace Engine