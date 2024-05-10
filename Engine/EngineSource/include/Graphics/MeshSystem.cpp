#include "Graphics/MeshSystem.h"



std::mutex Engine::MeshSystem::mutex_;
Engine::MeshSystem* Engine::MeshSystem::pInstance = nullptr;

Engine::MeshSystem::Instance* Engine::MeshSystem::intersect(const ray& r, hitInfo& hInfo)
{
	Instance* tempInst1 = hologramGroup.intersect(r, hInfo);
	Instance* tempInst2 = normVisGroup.intersect(r, hInfo);

	return tempInst1 != nullptr ? tempInst1 : tempInst2;
}

void Engine::MeshSystem::render()
{
	normVisGroup.render();
	hologramGroup.render();
}

Engine::MeshSystem* Engine::MeshSystem::Init()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (pInstance == nullptr)
	{
		pInstance = new MeshSystem();
	}

	return pInstance;
}

void Engine::MeshSystem::Deinit()
{
	delete pInstance;
}

Engine::MeshSystem::MeshSystem()
{
	D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TC", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TOWORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TOWORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TOWORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TOWORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	D3D_SHADER_MACRO pm[] = { "FIRST_SHADER", "1", NULL, NULL };

	auto NormalVis = Engine::ShaderManager::CompileAndCreateShader("NormalVis", L"Shaders\\VertexShader.hlsl", 
																			L"Shaders\\PixelShader.hlsl", ied, 9u, nullptr, pm);
	if (!NormalVis)
		throw std::runtime_error("Failed to compile and create shader!");

	auto HologramGroup = Engine::ShaderManager::CompileAndCreateShader("HologramGroup", L"Shaders\\Hologram.shader",
		L"Shaders\\Hologram.shader", ied, 9u, nullptr, pm, "vsMain", "psMain");
	if (!HologramGroup)
		throw std::runtime_error("Failed to compile and create shader!");

	normVisGroup.updateShader(NormalVis);
	hologramGroup.updateShader(HologramGroup);
}
