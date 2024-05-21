#include "Graphics/MeshSystem.h"



std::mutex Engine::MeshSystem::mutex_;
Engine::MeshSystem* Engine::MeshSystem::pInstance = nullptr;

std::vector<Engine::mat4*> Engine::MeshSystem::intersect(const ray& r, hitInfo& hInfo)
{
	auto firstPair = hologramGroup.intersect(r, hInfo);
	auto secondPair = normVisGroup.intersect(r, hInfo);
	
	std::vector<mat4*> transforms;
	if (secondPair.modelIndex != -1)
	{
		normVisGroup.getInstanceTransform(secondPair.modelIndex, secondPair.perMaterialIndex, secondPair.materialIndex, transforms);
	}
	else if (firstPair.modelIndex != -1)
	{
		hologramGroup.getInstanceTransform(firstPair.modelIndex, firstPair.perMaterialIndex, firstPair.materialIndex, transforms);
	}

	return transforms;
}

void Engine::MeshSystem::updateInstanceBuffers()
{
	normVisGroup.updateInstanceBuffers();
	hologramGroup.updateInstanceBuffers();
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

	auto NormalVisColor = Engine::ShaderManager::CompileAndCreateShader("NormalVisColor", L"Shaders\\normalColor\\VertexShader.hlsl", 
																			L"Shaders\\normalColor\\PixelShader.hlsl", ied, 9u, nullptr, pm);

	auto NormalVisLines = Engine::ShaderManager::CompileAndCreateShader("NormalVisLines", L"Shaders\\normalLines\\VertexShader.hlsl",
		L"Shaders\\normalLines\\PixelShader.hlsl", L"Shaders\\normalLines\\HullShader.hlsl", L"Shaders\\normalLines\\DomainShader.hlsl",
		L"Shaders\\normalLines\\GSnormal.hlsl", ied, 9u, nullptr, pm, D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	NormalVisLines->DisableShader();
	if (!NormalVisColor)
		throw std::runtime_error("Failed to compile and create shader!");

	auto HologramGroup = Engine::ShaderManager::CompileAndCreateShader("HologramGroup", L"Shaders\\Hologram.shader",
		L"Shaders\\Hologram.shader", L"Shaders\\HullShader.hlsl", L"Shaders\\DomainShader.hlsl", L"Shaders\\GSHologram.hlsl", ied, 9u, nullptr, pm, D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST, "vsMain", "psMain");
	if (!HologramGroup)
		throw std::runtime_error("Failed to compile and create shader!");

	normVisGroup.addShader(NormalVisLines);
	normVisGroup.addShader(NormalVisColor);
	hologramGroup.addShader(HologramGroup);
	hologramGroup.addShader(NormalVisLines);
}
