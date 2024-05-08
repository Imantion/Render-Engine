#pragma once
#include <vector>
#include <memory>
#include "Math/matrix.h"
#include "Math/math.h"
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
			vec3 color;
		};

		struct Instance
		{
			mat4 tranformation;
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
	public:

		void addModel(std::shared_ptr<Model> model, const vec3& position, const vec3& color)
		{

			Instance transform{ transformMatrix(position, vec3(0.0f, 0.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)) };
			auto it = perModel.end();
			for (auto i = perModel.begin(); i != perModel.end(); i++)
			{
				if (i->model.get() == model.get())
					it = i;
			}

			if (it == perModel.end())
			{
				std::vector<Instance> inst(1, transform);
				Material mat = { color };
				PerMaterial perMat = { mat,inst };

				PerMesh perMes = { std::vector<PerMaterial>(1,perMat) };
				
				PerModel perMod = { model, std::vector<PerMesh>(model->m_meshes.size(),perMes) };

				perModel.push_back(perMod);
			}
			else
			{
				auto pModel = it;
				for (uint32_t meshIndex = 0; meshIndex < pModel->perMesh.size(); ++meshIndex)
				{
					const Mesh& mesh = pModel->model->m_meshes[meshIndex];

					bool inserted = false;
					for (auto& perMaterial : pModel->perMesh[meshIndex].perMaterial)
					{
						if (color == perMaterial.material.color)
						{
							perMaterial.instances.push_back(transform);
							inserted = true;
						}
					}

					if (!inserted)
					{
						std::vector<Instance> inst;
						Material mat = { color };
						inst.push_back(transform);
						pModel->perMesh[meshIndex].perMaterial.push_back(PerMaterial{ mat, inst });
					}
				}
			}

		}

		void updateInstanceBuffers()
		{
			meshData.create();

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

						uint32_t numModelInstances = (uint32_t)instances.size();
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
			/*instanceBuffer.bind(2u);*/
			d3d->GetContext()->VSSetConstantBuffers(3u, 1, meshData.m_constBuffer.GetAddressOf());

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

					/*MeshData meshToModel = { * };*/
					meshData.updateBuffer(reinterpret_cast<const MeshData*>(mesh.instances.data())); // ... update shader local per-mesh uniform buffer

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
	public:
		OpaqueInstances opaqueInstances;

		void render();
		

		static MeshSystem* Init();
		

		static void Deinit();

	protected:
		static std::mutex mutex_;
		static MeshSystem* pInstance;
	};

}