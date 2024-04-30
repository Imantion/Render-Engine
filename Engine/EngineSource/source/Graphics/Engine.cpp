#include "Graphics/Engine.h"
#include "Graphics/ShaderManager.h"
#include "Window/Window.h"
#include <assert.h>


bool Engine::Engine::isInitialized = false;

void Engine::Engine::Init()
{
	D3D::Init();
	isInitialized = true;
}

void Engine::Engine::Deinit()
{
	D3D::GetInstance()->Reset();
}
