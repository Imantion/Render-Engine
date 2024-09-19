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

		TransformSystem(const TransformSystem&) = delete;
		void operator=(const TransformSystem&) = delete;

		struct transforms
		{
			mat4 modelToWold;
		};
		
		static TransformSystem* Init();
		static void Deinit();

		uint32_t AddModelTransform(std::vector<transforms>& transf);
		uint32_t AddModelTransform(const transforms& transf, uint32_t meshesAmount);

		std::vector<transforms>& GetModelTransforms(uint32_t ID);
		bool CheckForExistance(uint32_t ID);

		void RemoveModelTransform(uint32_t id);
		void SetModelPosition(uint32_t id, const std::vector<vec3>& position);
		void SetModelMeshPosition(uint32_t id, uint32_t meshIndex, const vec3& position);
		void ScaleModelTransform(uint32_t id, float radius);
		void TranslateModel(uint32_t id, vec3 position);

	protected:
		TransformSystem() = default;

		~TransformSystem() = default;

	private:
		static TransformSystem* m_instance;
		SolidVector<std::vector<transforms>> m_transforms;
		static std::mutex m_mutex;

	};
}

