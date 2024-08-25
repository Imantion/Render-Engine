#pragma once
#include "Graphics/D3D.h"
#include "Graphics/Instances.h"
#include "Graphics/Materials.h"
#include "Graphics/Buffers.h"
#include <mutex>
#include <vector>

namespace Engine
{
	struct mat4;
	struct shader;

	class DecalSystem
	{
	public:

		DecalSystem(const DecalSystem& other) = delete;
		DecalSystem& operator=(const DecalSystem& other) = delete;

		void UpdateBuffer();

		void AddDecal(const Materials::DecalMaterial& material, const mat4& decalToWorld, uint32_t objectID);
		void Draw();

		void SetShader(std::shared_ptr<shader> shader);

		static DecalSystem* Init();
		static void Deinit();

	private:
		DecalSystem();

		struct DecalData
		{
			mat4 transform;
			uint32_t objectID;
			uint32_t objectTransformID;
			vec3 relPosition;

		};
		struct PerTexture
		{
			Materials::DecalMaterial material;
			std::vector<DecalData> instances;

			bool operator==(const Materials::DecalMaterial& other) const
			{
				return material == other;
			}
		};

	private:
		static DecalSystem* m_instance;
		static std::mutex m_mutex;

		std::shared_ptr<shader> m_shader;
		std::vector<PerTexture> m_perTexture;
		IndexBuffer m_indexBuffer;
		VertexBuffer<Instances::DecalInstance> m_instanceBuffer;
	};
}