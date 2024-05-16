#pragma once
#include "Graphics/D3D.h"

namespace Engine
{
	class Window;

	class Engine
	{
	public:
		static void Init();
		static void Deinit();

	private:
		static bool isInitialized;

	};
}