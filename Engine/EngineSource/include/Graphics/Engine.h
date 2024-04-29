#pragma once
#include "Graphics/D3D.h"

struct ConstantBuffer
{
	float g_resolution[4];
	float g_time;
	float padding[3];
};

namespace Engine
{
	class Window;

	class Engine
	{
	public:
		static void Init();
		static void Deinit();

		static void PrepareTriangle(D3D_SHADER_MACRO* psMacro);
		static void PrepareCurlesque();
	

	private:
		static bool isInitialized;

	};
}