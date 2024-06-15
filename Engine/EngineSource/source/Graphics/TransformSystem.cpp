#include "Graphics/TransformSystem.h"


Engine::TransformSystem* Engine::TransformSystem::m_Instance;
std::mutex Engine::TransformSystem::m_mutex;

Engine::TransformSystem* Engine::TransformSystem::Init()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (m_Instance == nullptr)
	{
		m_Instance = new TransformSystem();
	}

	return m_Instance;
}

void Engine::TransformSystem::Deinit()
{
	delete m_Instance;
}

uint32_t Engine::TransformSystem::AddModelTransform(std::vector<transforms>& transf)
{
	return m_transforms.insert(transf);
}

uint32_t Engine::TransformSystem::AddModelTransform(transforms& transf, uint32_t meshesAmount)
{
	return m_transforms.insert(std::vector<transforms>(meshesAmount, transf));
}

void Engine::TransformSystem::RemoveModelTransform(uint32_t id)
{
	m_transforms.erase(id);
}

void Engine::TransformSystem::SetModelPosition(uint32_t id, vec3 position)
{
	auto transformations = m_transforms.at(id);

	for (size_t i = 0; i < transformations.size(); i++)
	{
		reinterpret_cast<vec3&>(*transformations[i].modelToWold[3]) = position;
	}
}

void Engine::TransformSystem::TranslateModel(uint32_t id, vec3 position)
{
	auto transformations = m_transforms.at(id);

	for (size_t i = 0; i < transformations.size(); i++)
	{
		transformations[i].modelToWold[3][0] += position.x;
		transformations[i].modelToWold[3][1] += position.y;
		transformations[i].modelToWold[3][2] += position.z;
	}
}
