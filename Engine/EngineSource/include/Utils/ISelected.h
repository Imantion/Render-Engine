#pragma once
#include "Math/math.h"
#include "Graphics/MeshSystem.h"

namespace Engine
{

	class ISelected
	{
	public:

		virtual void update(void* instances, uint32_t size) = 0;

		virtual ~ISelected() = default;
	};

	template <typename I> 
	class IInstanceSelected : public ISelected
	{
	public:
		IInstanceSelected(uint32_t transformId, std::vector<I*>&& instances);
		
		virtual void update(void* instances, uint32_t size)
		{
			I* typedInstances = static_cast<I*>(instances);

			for (size_t i = 0; i < size; i++)
			{
				*m_instances[i] = typedInstances[i];
			}

			MeshSystem::Init()->updateInstanceBuffers();
		}

	private:
		std::vector<I*> m_instances;
		uint32_t m_transformId;
	};

	template<typename I>
	inline IInstanceSelected<I>::IInstanceSelected(uint32_t transformId, std::vector<I*>&& instances) : m_transformId(transformId)
	{
		m_instances = instances;
	}
}

