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
	private:
		void* m_handle = nullptr;

	};

} // namespace Engine