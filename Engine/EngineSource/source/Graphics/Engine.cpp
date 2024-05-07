#include "Graphics/Engine.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Renderer.h"
#include "Graphics/Model.h"
#include "Window/Window.h"
#include <assert.h>


bool Engine::Engine::isInitialized = false;

void Engine::Engine::Init()
{
	D3D::Init();
	Renderer::Init();
	ModelManager::Init();
	isInitialized = true;
}

void Engine::Engine::Deinit()
{
	D3D::GetInstance()->Reset();
	Renderer::Deinit();
	ModelManager::Deinit();
	ShaderManager::deleteShaders();
}
