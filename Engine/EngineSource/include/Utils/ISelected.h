#pragma once
#include "Math/math.h"
#include "Graphics/MeshSystem.h"

namespace Engine
{

	class ISelected
	{
	public:

		virtual void update(void* instances, uint32_t size) = 0;
		virtual void update(void* instance) = 0;

		uint32_t getTransformId() { return m_transformId; }

		virtual ~ISelected() = default;

	protected:
		uint32_t m_transformId;
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

		virtual void update(void* instances)
		{
			I* typedInstances = static_cast<I*>(instances);

			for (size_t i = 0; i < m_instances.size(); i++)
			{
				*m_instances[i] = typedInstances[0];
			}

			MeshSystem::Init()->updateInstanceBuffers();
		}

	private:
		std::vector<I*> m_instances;
	};

	template<typename I>
	inline IInstanceSelected<I>::IInstanceSelected(uint32_t transformId, std::vector<I*>&& instances)
	{
		m_transformId = transformId;
		m_instances = instances;
	}
}

