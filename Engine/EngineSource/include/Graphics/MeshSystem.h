#pragma once
#include <vector>
#include <memory>
#include "Math/matrix.h"
#include "Graphics/Buffers.h"
#include "Graphics/Model.h"

namespace Engine
{


	class OpaqueInstances
	{
	protected:

		struct MeshData
		{
			mat4 meshToModel;
		};
		struct Material
		{
			
		};

		struct Instance
		{
			
		};

		struct PerMaterial
		{
			Material material;
			std::vector<Instance> instances;
		};

		struct PerMesh
		{
			std::vector<PerMaterial> perMaterial;
		};

		struct PerModel
		{
			std::shared_ptr<Model> model;
			std::vector<PerMesh> perMesh;
		};

		std::vector<PerModel> perModel;
		VertexBuffer<Instance> instanceBuffer;
		ConstBuffer<MeshData> meshData;
		// ConstBuffer<MaterialData> materialData;

		void updateInstanceBuffers()
		{
			uint32_t totalInstances = 0;
			for (auto& perModel : perModel)
				for (auto& perMesh : perModel.perMesh)
					for (const auto& perMaterial : perMesh.perMaterial)
						totalInstances += uint32_t(perMaterial.instances.size());

			if (totalInstances == 0)
				return;

			instanceBuffer.create(totalInstances, D3D11_USAGE_DYNAMIC); // resizes if needed

			D3D11_MAPPED_SUBRESOURCE mapping;
			instanceBuffer.map(mapping);
			Instance* dst = static_cast<Instance*>(mapping.pData);

			uint32_t copiedNum = 0;
			for (const auto& perModel : perModel)
			{
				for (uint32_t meshIndex = 0; meshIndex < perModel.perMesh.size(); ++meshIndex)
				{
					const Mesh& mesh = perModel.model->m_meshes[meshIndex];

					for (const auto& perMaterial : perModel.perMesh[meshIndex].perMaterial)
					{
						auto& instances = perMaterial.instances;

						uint32_t numModelInstances = instances.size();
						for (uint32_t index = 0; index < numModelInstances; ++index)
						{
							dst[copiedNum++] = instances[index];
						}
					}
				}
			}

			instanceBuffer.unmap();
		}

	public:
		void render()
		{
			if (instanceBuffer.getSize() == 0)
				return;

			D3D* d3d = D3D::GetInstance();
			/*shader.bind();*/
			instanceBuffer.bind();

			uint32_t renderedInstances = 0;
			for (const auto& perModel : perModel)
			{
				if (perModel.model.get() == nullptr) continue;

				perModel.model->m_vertices.bind();
				perModel.model->m_indices.bind();

				for (uint32_t meshIndex = 0; meshIndex < perModel.perMesh.size(); ++meshIndex)
				{
					const Mesh& mesh = perModel.model->m_meshes[meshIndex];
					const auto& meshRange = perModel.model->m_ranges[meshIndex];

					MeshData meshToModel = { *mesh.instances.data() };
					meshData.updateBuffer(&meshToModel); // ... update shader local per-mesh uniform buffer

					for (const auto& perMaterial : perModel.perMesh[meshIndex].perMaterial)
					{
						if (perMaterial.instances.empty()) continue;

						const auto& material = perMaterial.material;

						// ... update shader local per-draw uniform buffer
						// materialData.update(...); // we don't have it in HW4

						// ... bind each material texture, we don't have it in HW4

						uint32_t numInstances = uint32_t(perMaterial.instances.size());
						d3d->GetContext()->DrawIndexedInstanced(meshRange.indexNum, numInstances, meshRange.indexOffset, meshRange.vertexOffset, renderedInstances);
						renderedInstances += numInstances;
					}
				}
			}
		}
	};

	class MeshSystem
	{
		OpaqueInstances opaqueInstances;

		void render()
		{
			opaqueInstances.render();
		}
	};

}