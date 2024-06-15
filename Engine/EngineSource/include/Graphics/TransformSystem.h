#pragma once
#include "Utils/solid_vector.h"
#include "Math/matrix.h"
#include "Math/math.h"
#include <memory>
#include <mutex>

namespace Engine
{
	class TransformSystem
	{
	public:
		struct transforms
		{
			mat4 modelToWold;
		};
		
		static TransformSystem* Init();
		static void Deinit();

		uint32_t AddModelTransform(std::vector<transforms>& transf);
		uint32_t AddModelTransform(transforms& transf, uint32_t meshesAmount);

		void RemoveModelTransform(uint32_t id);
		void SetModelPosition(uint32_t id, vec3 position);
		void TranslateModel(uint32_t id, vec3 position);

	private:
		static TransformSystem* m_Instance;
		SolidVector<std::vector<transforms>> m_transforms;
		static std::mutex m_mutex;

	};
}

