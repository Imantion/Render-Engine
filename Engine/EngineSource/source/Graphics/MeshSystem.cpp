#include "Graphics/MeshSystem.h"



std::mutex Engine::MeshSystem::mutex_;
Engine::MeshSystem* Engine::MeshSystem::pInstance = nullptr;

uint32_t Engine::MeshSystem::intersect(const ray& r, hitInfo& hInfo)
{
	uint32_t first = hologramGroup.intersect(r, hInfo);
	uint32_t second = normVisGroup.intersect(r, hInfo);
	uint32_t third = textureGroup.intersect(r, hInfo);
	uint32_t firth = opaqueGroup.intersect(r, hInfo);
	if (firth != -1)
		return firth;
	return third != -1? third: (second != -1? second: first);
}

void Engine::MeshSystem::updateInstanceBuffers()
{
	normVisGroup.updateInstanceBuffers();
	hologramGroup.updateInstanceBuffers();
	textureGroup.updateInstanceBuffers();
	opaqueGroup.updateInstanceBuffers();
}

void Engine::MeshSystem::render()
{
	normVisGroup.render();
	hologramGroup.render();
	textureGroup.render();
	opaqueGroup.render();
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
	pInstance = nullptr;
}

