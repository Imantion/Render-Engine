#include "Graphics/MeshSystem.h"



std::mutex Engine::MeshSystem::mutex_;
Engine::MeshSystem* Engine::MeshSystem::pInstance = nullptr;

std::vector<Engine::mat4*> Engine::MeshSystem::intersect(const ray& r, hitInfo& hInfo)
{
	auto firstPair = hologramGroup.intersect(r, hInfo);
	auto secondPair = normVisGroup.intersect(r, hInfo);
	auto thirdPair = textureGroup.intersect(r, hInfo);
	
	std::vector<mat4*> transforms;
	if (thirdPair.modelIndex != -1)
	{
		textureGroup.getInstanceTransform(thirdPair.modelIndex, thirdPair.perMaterialIndex, thirdPair.materialIndex, transforms);
	}
	else if (secondPair.modelIndex != -1)
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
	textureGroup.updateInstanceBuffers();
}

void Engine::MeshSystem::render()
{
	normVisGroup.render();
	hologramGroup.render();
	textureGroup.render();
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

