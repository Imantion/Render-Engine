#include "Graphics/Engine.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Renderer.h"
#include "Graphics/Model.h"
#include "Graphics/MeshSystem.h"
#include "Graphics/TextureManager.h"
#include "Graphics/TransformSystem.h"
#include "Graphics/PostProcess.h"
#include "Graphics/LightSystem.h"
#include "Graphics/ShadowSystem.h"
#include "Graphics/ParticleSystem.h"
#include "Graphics/DecalSystem.h"
#include "Window/Window.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
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
	LightSystem::Init();
	ShadowSystem::Init();
	ParticleSystem::Init();
	DecalSystem::Init();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	ImGui_ImplDX11_Init(D3D::GetInstance()->GetDevice(), D3D::GetInstance()->GetContext());

	isInitialized = true;
}

void Engine::Engine::Deinit()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	D3D::GetInstance()->Reset();
	Renderer::Deinit();
	ModelManager::Deinit();
	MeshSystem::Deinit();
	ShaderManager::deleteShaders();
	TextureManager::Deinit();
	TransformSystem::Deinit();
	PostProcess::Deinit();
	LightSystem::Deinit();
	ShadowSystem::Deinit();
	ParticleSystem::Deinit();
	DecalSystem::Deinit();
}
