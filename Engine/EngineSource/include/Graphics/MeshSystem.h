#pragma once
#include <vector>
#include <memory>
#include "Math/matrix.h"
#include "Math/math.h"
#include "Graphics/Buffers.h"
#include "Graphics/Model.h"
#include "Graphics/ShaderManager.h"

namespace Engine
{
	template <typename I, typename M>
	class OpaqueInstances
	{
	protected:

		struct MeshData
		{
			mat4 meshToModel;
		};

		struct PerMaterial
		{
			M material;
			std::vector<I> instances;
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

		std::shared_ptr<shader> m_shader;
		std::vector<PerModel> perModel;
		VertexBuffer<I> instanceBuffer;
		ConstBuffer<MeshData> meshData;
		// ConstBuffer<MaterialData> materialData;
	public:

		void updateShader(std::shared_ptr<shader> shdr)
		{
			m_shader = shdr;
		}

		OpaqueInstances() { meshData.create(); }

		void addModel(std::shared_ptr<Model> model, const vec3& color, const vec3& position, float xRotation = 0.0f, float yRotation = 0.0f, float zRotation = 0.0f) // rotation order means!
		{
			float pi = 3.14159265359;
			auto rotX = mat4::rotateX(pi * (-xRotation) / 360.0f);
			auto rotY = mat4::rotateY(pi * (-yRotation) / 360.0f);
			auto rotZ = mat4::rotateZ(pi * (-zRotation) / 360.0f);
			auto trans = transformMatrix(position, vec3(0.0f, 0.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

			I transform{ rotX * rotY * rotZ * trans };
			auto it = perModel.end();
			for (auto i = perModel.begin(); i != perModel.end(); i++)
			{
				if (i->model.get() == model.get())
					it = i;
			}

			if (it == perModel.end())
			{
				std::vector<I> inst(1, transform);
				M mat = { color };
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
						std::vector<I> inst;
						M mat = { color };
						inst.push_back(transform);
						pModel->perMesh[meshIndex].perMaterial.push_back(PerMaterial{ mat, inst });
					}
				}
			}

		}

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
			I* dst = static_cast<I*>(mapping.pData);

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
			m_shader->BindShader();
			instanceBuffer.bind(1u);
			d3d->GetContext()->VSSetConstantBuffers(2u, 1, meshData.m_constBuffer.GetAddressOf());

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
		struct Material
		{
			vec3 color;
		};

		struct Instance
		{
			mat4 tranformation;
		};

		OpaqueInstances<Instance, Material> hologramGroup;
		OpaqueInstances<Instance, Material> normVisGroup;

		void render();


		static MeshSystem* Init();


		static void Deinit();
	private:
		MeshSystem();

	protected:
		static std::mutex mutex_;
		static MeshSystem* pInstance;
	};

}