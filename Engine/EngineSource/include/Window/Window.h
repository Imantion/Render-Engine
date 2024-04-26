#pragma once
#include "Windows.h"
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
		void Resize(int wWidth, int wHeight);
		void ResizeFrameBuffer(int wWidth, int wHeight);
		bool isClosed() const;

		void flush();
		void clearScreen();
		void setPixel(int x, int y);
		
		float getAspectRation() const { return aspectRatio; }
		void* getMemoryBuffer() const { return buffer.memory.get(); }
		int getWindowHeight() const { return height; }
		int getWindowWidth() const { return width; }
		int getBufferHeight() const { return buffer.height; }
		int getBufferWidth() const { return buffer.width; }
		HWND getHWND() { return (HWND)m_handle; }

		bool wasWindowResized();
	private:

		void* m_handle = nullptr;
		HDC device_context;
		int height;
		int width;
		int buffersize;
		bool closed;
		bool wasResized;
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