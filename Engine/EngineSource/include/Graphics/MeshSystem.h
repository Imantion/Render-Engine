#pragma once
#include <vector>
#include <memory>
#include "Math/matrix.h"
#include "Graphics/Materials.h"
#include "Math/hitable.h"
#include "Graphics/Buffers.h"
#include "Graphics/Model.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/TextureManager.h"
#include "Graphics/TransformSystem.h"

namespace Engine
{

	template<typename M>
	struct MaterialDataType {
		using type = M;
	};

	template<>
	struct MaterialDataType<Materials::OpaqueTextureMaterial> {
		using type = vec4;
	};

	template <typename I, typename M>
	class OpaqueInstances
	{
	protected:

		struct instanceOfModel
		{
			int modelIndex;
			int perMaterialIndex;
			int materialIndex;
		};

		struct instanceBufferData
		{
			TransformSystem::transforms transformData;
			I instanceData;
		};

		struct PerInstance
		{
			uint32_t transformsId;
			I instanceData;
		};

		struct MeshData
		{
			mat4 meshToModel;
		};

		using MaterialType = typename MaterialDataType<M>::type;

		struct MaterialData
		{
			MaterialType material;
		};

		struct PerMaterial
		{
			M material;
			std::vector<PerInstance> instances;
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

		std::vector<std::shared_ptr<shader>> m_shaders;
		std::vector<PerModel> perModel;
		VertexBuffer<instanceBufferData> instanceBuffer;
		ConstBuffer<MeshData> meshData;
		ConstBuffer<MaterialData> materialData;
	public:

		void addShader(std::shared_ptr<shader> shdr)
		{
			m_shaders.push_back(shdr);
		}

		OpaqueInstances() { meshData.create(); materialData.create(); }

		uint32_t intersect(const ray& r, hitInfo& hInfo)
		{
			uint32_t transformId = -1;
			auto TS = TransformSystem::Init();
			ray transformedRay = r;
			for (size_t i = 0; i < perModel.size(); i++)
			{
				for (uint32_t meshIndex = 0; meshIndex < perModel[i].perMesh.size(); ++meshIndex)
				{
					const Mesh& mesh = perModel[i].model->m_meshes[meshIndex];

					for (size_t j = 0; j < perModel[i].perMesh[meshIndex].perMaterial.size(); j++)
					{
						auto& instances = perModel[i].perMesh[meshIndex].perMaterial[j].instances;

						uint32_t numModelInstances = (uint32_t)instances.size();
						for (uint32_t index = 0; index < numModelInstances; ++index)
						{
							uint32_t currentId = instances[index].transformsId;
							auto& meshInstanceTransform = TS->GetModelTransforms(currentId)[meshIndex].modelToWold;

							transformedRay.origin = vec4(r.origin, 1.0f) * mat4::Inverse(meshInstanceTransform) * mesh.invInstances[0];
							transformedRay.direction = vec4(r.direction, 0.0f) * mat4::Inverse(meshInstanceTransform)  * mesh.invInstances[0];
							
							if (mesh.intersect(transformedRay, hInfo))
							{
								transformId = currentId;
								hInfo.p = vec4(transformedRay.point_at_parameter(hInfo.t),1.0f) * mesh.instances[0] * meshInstanceTransform;
								/*hInfo.p = r.point_at_parameter(hInfo.t);*/
							}
						}
					}
				}
			}

			return transformId;
		}


		uint32_t addModel(std::shared_ptr<Model> model, const M& material, const TransformSystem::transforms& modelTransforms, const I& instance = {}) // returns model transform ID
		{
			auto TS = TransformSystem::Init();
			uint32_t modelTransformsId = TS->AddModelTransform(modelTransforms, (uint32_t)model->m_meshes.size());

			auto it = perModel.end();
			for (auto i = perModel.begin(); i != perModel.end(); i++)
			{
				if (i->model.get() == model.get())
					it = i;
			}

			if (it == perModel.end())
			{
				std::vector<PerInstance> inst(1, PerInstance{modelTransformsId, instance});

				PerMaterial perMat = { material,inst };

				PerMesh perMes = { std::vector<PerMaterial>(1,perMat) };

				PerModel perMod = { model, std::vector<PerMesh>(model->m_meshes.size(),perMes) };

				perModel.push_back(perMod);
			}
			else
			{
				auto pModel = it;
				for (uint32_t meshIndex = 0; meshIndex < pModel->perMesh.size(); ++meshIndex)
				{

					bool inserted = false;
					for (auto& perMaterial : pModel->perMesh[meshIndex].perMaterial)
					{
						if (material == perMaterial.material)
						{
							perMaterial.instances.push_back(PerInstance{ modelTransformsId, instance });
							inserted = true;
						}
					}

					if (!inserted)
					{
						std::vector<PerInstance> inst;
						inst.push_back(PerInstance{ modelTransformsId, instance });
						pModel->perMesh[meshIndex].perMaterial.push_back(PerMaterial{ material, inst });
					}
				}
			}
			return modelTransformsId;
		}
		
		uint32_t addModel(std::shared_ptr<Model> model, const std::vector<M>& material, const TransformSystem::transforms& modelTransforms, const I& instance = {}) // returns model transform ID
		{
			auto TS = TransformSystem::Init();
			uint32_t modelTransformsId = TS->AddModelTransform(modelTransforms, (uint32_t)model->m_meshes.size());

			auto it = perModel.end();
			for (auto i = perModel.begin(); i != perModel.end(); i++)
			{
				if (i->model.get() == model.get())
					it = i;
			}

			PerModel perMod;
			if (it == perModel.end())
			{
				perMod.perMesh.resize(model->m_meshes.size());
				perMod.model = model;

				for (size_t i = 0; i < model->m_meshes.size(); i++)
				{
					std::vector<PerInstance> inst(1, PerInstance{ modelTransformsId, instance });
					PerMaterial perMat = { material[i],inst};
					PerMesh perMes = { std::vector<PerMaterial>(1,perMat) };
					
					perMod.perMesh[i] = perMes;
				}

				perModel.push_back(perMod);
			}
			else
			{
				auto pModel = it;
				for (uint32_t meshIndex = 0; meshIndex < pModel->perMesh.size(); ++meshIndex)
				{

					bool inserted = false;
					for (auto& perMaterial : pModel->perMesh[meshIndex].perMaterial)
					{
						if (material[meshIndex] == perMaterial.material)
						{
							perMaterial.instances.push_back(PerInstance{ modelTransformsId, instance });
							inserted = true;
						}
					}

					if (!inserted)
					{
						std::vector<PerInstance> inst;
						inst.push_back(PerInstance{ modelTransformsId, instance });
						pModel->perMesh[meshIndex].perMaterial.push_back(PerMaterial{ material[meshIndex], inst});
					}
				}
			}
			return modelTransformsId;
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
			instanceBufferData* dst = static_cast<instanceBufferData*>(mapping.pData);

			uint32_t copiedNum = 0;
			auto TS = TransformSystem::Init();
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
							dst[copiedNum++] = instanceBufferData{ TS->GetModelTransforms(instances[index].transformsId)[meshIndex],  instances[index].instanceData};
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
			for (size_t i = 0; i < m_shaders.size(); i++)
			{
				if (!m_shaders[i]->isEnabled)
					continue;
				m_shaders[i]->BindShader();
				instanceBuffer.bind(1u);
				meshData.bind(2u, shaderTypes::VS);

				materialData.bind(2u, shaderTypes::PS);
				


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

						meshData.updateBuffer(reinterpret_cast<const MeshData*>(mesh.instances.data())); // ... update shader local per-mesh uniform buffer

						for (const auto& perMaterial : perModel.perMesh[meshIndex].perMaterial)
						{
							if (perMaterial.instances.empty()) continue;

							const auto& material = perMaterial.material;
							MaterialData data = { material };
							// ... update shader local per-draw uniform buffer
							materialData.updateBuffer(&data);


							uint32_t numInstances = uint32_t(perMaterial.instances.size());
							d3d->GetContext()->DrawIndexedInstanced(meshRange.indexNum, numInstances, meshRange.indexOffset, meshRange.vertexOffset, renderedInstances);
							renderedInstances += numInstances;
						}
					}
				}
			}
			}
			
	};


	class MeshSystem
	{
	public:

		struct Instance 
		{
		};

		struct EmmisiveInstance
		{
			vec3 emmisiveColor;
		};

		OpaqueInstances<Instance, Materials::HologramMaterial> hologramGroup;
		OpaqueInstances<Instance, Materials::NormVisMaterial> normVisGroup;
		OpaqueInstances<Instance, Materials::TextureMaterial> textureGroup;
		OpaqueInstances<Instance, Materials::OpaqueTextureMaterial> opaqueGroup;
		OpaqueInstances<EmmisiveInstance, Materials::EmmisiveMaterial> emmisiveGroup;

		int intersect(const ray& r, hitInfo& hInfo);

		void updateInstanceBuffers();
		void render();


		static MeshSystem* Init();


		static void Deinit();
	private:
		MeshSystem() = default;

	protected:
		static std::mutex mutex_;
		static MeshSystem* pInstance;
	};

	template <>
	inline void OpaqueInstances<MeshSystem::Instance, Materials::OpaqueTextureMaterial>::render();


	
}

