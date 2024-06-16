#include "Graphics/Engine.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Renderer.h"
#include "Graphics/Model.h"
#include "Graphics/MeshSystem.h"
#include "Graphics/TextureManager.h"
#include "Graphics/TransformSystem.h"
#include "Graphics/PostProcess.h"
#include "Window/Window.h"
#include <assert.h>


bool Engine::Engine::isInitialized = false;

void Engine::Engine::Init()
{
	D3D::Init();
	Renderer::Init();
	ModelManager::Init();
	MeshSystem::Init();
	TextureManager::Init();
	TransformSystem::Init();
	PostProcess::Init();

	isInitialized = true;
}

void Engine::Engine::Deinit()
{
	D3D::GetInstance()->Reset();
	Renderer::Deinit();
	ModelManager::Deinit();
	MeshSystem::Deinit();
	ShaderManager::deleteShaders();
	TextureManager::Deinit();
	TransformSystem::Deinit();
	PostProcess::Deinit();
}
