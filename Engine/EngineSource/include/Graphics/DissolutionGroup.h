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
#include "Graphics/Instances.h"
#include "Graphics/DecalSystem.h"

namespace Engine
{
	template <typename I>
	class OpaqueInstances<I, Materials::DissolutionMaterial>
	{
	protected:

		friend class MeshSystem;

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
			uint32_t instanceID;
		};

		struct PerInstance
		{
			uint32_t transformsId;
			I instanceData;
			uint32_t instanceMeshId = g_meshIdGenerator++;
		};

		struct MeshData
		{
			mat4 meshToModel;
		};

		struct MaterialData
		{
			vec4 material;
		};

		struct PerMaterial
		{
			Materials::DissolutionMaterial material;
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
		std::shared_ptr<shader> GBufferShader;
		std::vector<PerModel> perModel;
		VertexBuffer<instanceBufferData> instanceBuffer;
		ConstBuffer<MeshData> meshData;
		ConstBuffer<MaterialData> materialData;
	public:
		struct ModelInstanceData
		{
			std::shared_ptr<Model> model;
			std::vector<Materials::DissolutionMaterial> material;
			std::vector<PerInstance> instance;
		};

	public:

		void addShader(std::shared_ptr<shader> shdr)
		{
			m_shaders.push_back(shdr);
		}

		void setGBufferShader(std::shared_ptr<shader> shdr)
		{
			GBufferShader = shdr;
		}

		OpaqueInstances() { meshData.create(); materialData.create(); }

		std::vector<I*> getInstanceByTransformId(uint32_t transformId)
		{
			std::vector<I*> modelInstanes;

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
							if (transformId == instances[index].transformsId)
								modelInstanes.push_back(&instances[index].instanceData);
						}
					}
				}
			}

			return modelInstanes;
		}

		int getTrasnformIdByInstanceMeshId(uint32_t meshId)
		{
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
							if (meshId == instances[index].instanceMeshId)
								return instances[index].transformsId;
						}
					}
				}
			}

			return -1;
		}

		int intersect(const ray& r, hitInfo& hInfo)
		{
			int transformId = -1;
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
							transformedRay.direction = vec4(r.direction, 0.0f) * mat4::Inverse(meshInstanceTransform) * mesh.invInstances[0];

							if (mesh.intersect(transformedRay, hInfo))
							{
								transformId = currentId;
								hInfo.p = vec4(transformedRay.point_at_parameter(hInfo.t), 1.0f) * mesh.instances[0] * meshInstanceTransform;
								/*hInfo.p = r.point_at_parameter(hInfo.t);*/
							}
						}
					}
				}
			}

			return transformId;
		}

		int intersectMesh(const ray& r, hitInfo& hInfo)
		{
			int meshIsntanceId = -1;
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
							uint32_t meshId = instances[index].instanceMeshId;
							auto& meshInstanceTransform = TS->GetModelTransforms(currentId)[meshIndex].modelToWold;

							transformedRay.origin = vec4(r.origin, 1.0f) * mat4::Inverse(meshInstanceTransform) * mesh.invInstances[0];
							transformedRay.direction = vec4(r.direction, 0.0f) * mat4::Inverse(meshInstanceTransform) * mesh.invInstances[0];

							if (mesh.intersect(transformedRay, hInfo))
							{
								meshIsntanceId = meshId;
								hInfo.p = vec4(transformedRay.point_at_parameter(hInfo.t), 1.0f) * mesh.instances[0] * meshInstanceTransform;
							}
						}
					}
				}
			}

			return meshIsntanceId;
		}

		ModelInstanceData removeByTransformId(uint32_t transformId, bool deleteTransform = true)
		{
			ModelInstanceData mData;

			for (int i = 0; i < perModel.size(); i++)
			{
				for (int meshIndex = 0; meshIndex < perModel[i].perMesh.size(); ++meshIndex)
				{
					const Mesh& mesh = perModel[i].model->m_meshes[meshIndex];

					for (int j = 0; j < perModel[i].perMesh[meshIndex].perMaterial.size(); j++)
					{
						auto& instances = perModel[i].perMesh[meshIndex].perMaterial[j].instances;

						for (int index = 0; index < instances.size(); ++index)
						{
							if (transformId == instances[index].transformsId)
							{
								mData.model = perModel[i].model;
								mData.material.push_back(perModel[i].perMesh[meshIndex].perMaterial[j].material);
								mData.instance.push_back(instances[index]);

								instances.erase(instances.begin() + index);
								--index;
							}
						}

						if (instances.size() == 0)
						{
							perModel[i].perMesh[meshIndex].perMaterial.erase(perModel[i].perMesh[meshIndex].perMaterial.begin() + j);
							--j;
						}
					}

					if (perModel[i].perMesh[meshIndex].perMaterial.size() == 0)
					{
						perModel[i].perMesh.erase(perModel[i].perMesh.begin() + meshIndex);
						--meshIndex;
					}
				}

				if (perModel[i].perMesh.size() == 0)
				{
					perModel.erase(perModel.begin() + i);
					--i;
				}
			}

			if (deleteTransform)
				TransformSystem::Init()->RemoveModelTransform(transformId);

			return mData;
		}

		void addModel(const ModelInstanceData& data)
		{
			auto it = perModel.end();
			for (auto i = perModel.begin(); i != perModel.end(); i++)
			{
				if (i->model.get() == data.model.get())
					it = i;
			}

			if (it == perModel.end())
			{
				PerModel perMod;
				perMod.model = data.model;
				for (size_t i = 0; i < data.model->m_meshes.size(); i++)
				{
					std::vector<PerInstance> inst(1, data.instance[i]);
					PerMaterial perMat = { data.material[i],inst };
					PerMesh perMes = { std::vector<PerMaterial>(1,perMat) };
					perMod.perMesh.push_back(perMes);
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
						if (data.material[meshIndex] == perMaterial.material)
						{
							perMaterial.instances.push_back(data.instance[meshIndex]);
							inserted = true;
						}
					}

					if (!inserted)
					{
						std::vector<PerInstance> inst;
						inst.push_back(data.instance[meshIndex]);
						pModel->perMesh[meshIndex].perMaterial.push_back(PerMaterial{ data.material[meshIndex], inst });
					}
				}
			}
		}

		uint32_t addModel(std::shared_ptr<Model> model, const Materials::DissolutionMaterial& material, uint32_t modelTransformsId, const I& instance) // returns model transform ID
		{

			auto it = perModel.end();
			for (auto i = perModel.begin(); i != perModel.end(); i++)
			{
				if (i->model.get() == model.get())
					it = i;
			}

			if (it == perModel.end())
			{
				std::vector<PerInstance> inst(1, PerInstance{ modelTransformsId, instance, g_meshIdGenerator++ });

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
							perMaterial.instances.push_back(PerInstance{ modelTransformsId, instance, g_meshIdGenerator++ });
							inserted = true;
						}
					}

					if (!inserted)
					{
						std::vector<PerInstance> inst;
						inst.push_back(PerInstance{ modelTransformsId, instance, g_meshIdGenerator++ });
						pModel->perMesh[meshIndex].perMaterial.push_back(PerMaterial{ material, inst });
					}
				}
			}
			return modelTransformsId;
		}

		uint32_t addModel(std::shared_ptr<Model> model, const std::vector<Materials::DissolutionMaterial>& material, uint32_t modelTransformsId, const I& instance) // returns model transform ID
		{

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
					std::vector<PerInstance> inst(1, PerInstance{ modelTransformsId, instance, g_meshIdGenerator++ });
					PerMaterial perMat = { material[i],inst };
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
							perMaterial.instances.push_back(PerInstance{ modelTransformsId, instance, g_meshIdGenerator++ });
							inserted = true;
						}
					}

					if (!inserted)
					{
						std::vector<PerInstance> inst;
						inst.push_back(PerInstance{ modelTransformsId, instance, g_meshIdGenerator++ });
						pModel->perMesh[meshIndex].perMaterial.push_back(PerMaterial{ material[meshIndex], inst });
					}
				}
			}
			return modelTransformsId;
		}

		uint32_t addModel(std::shared_ptr<Model> model, const Materials::DissolutionMaterial& material, const TransformSystem::transforms& modelTransforms, const I& instance)
		{
			auto TS = TransformSystem::Init();
			uint32_t modelTransformsId = TS->AddModelTransform(modelTransforms, (uint32_t)model->m_meshes.size());

			return addModel(model, material, modelTransformsId, instance);
		}


		uint32_t addModel(std::shared_ptr<Model> model, const std::vector<Materials::DissolutionMaterial>& material, const TransformSystem::transforms& modelTransforms, const I& instance)
		{
			auto TS = TransformSystem::Init();
			uint32_t modelTransformsId = TS->AddModelTransform(modelTransforms, (uint32_t)model->m_meshes.size());

			return addModel(model, material, modelTransformsId, instance);
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
							dst[copiedNum++] = instanceBufferData{ TS->GetModelTransforms(instances[index].transformsId)[meshIndex],  instances[index].instanceData, instances[index].instanceMeshId};
						}
					}
				}
			}

			instanceBuffer.unmap();
		}

		void update(float time)
		{
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
							instances[index].instanceData.passedTime += time;
						}
					}
				}
			}

			updateInstanceBuffers();
		}

		std::vector<uint32_t> checkTimer()
		{
			std::vector<uint32_t> ids;

			for (size_t i = 0; i < perModel.size(); i++)
			{
				auto& instances = perModel[i].perMesh[0].perMaterial[0].instances;

				uint32_t numModelInstances = (uint32_t)instances.size();
				for (uint32_t index = 0; index < numModelInstances; ++index)
				{
					if (instances[index].instanceData.passedTime >= instances[index].instanceData.animationDuration)
						ids.push_back(instances[index].transformsId);
				}
			}

			return ids;
		}

	public:

		void render()
		{
			for (size_t i = 0; i < m_shaders.size(); i++)
			{
				if (m_shaders[i]->isEnabled)
					renderUsingShader(m_shaders[i]);

			}
		}

		void renderUsingShader(std::shared_ptr<shader> shaderToRender)
		{
			// Custom render implementation for TextureMaterial
			if (instanceBuffer.getSize() == 0)
				return;

			D3D* d3d = D3D::GetInstance();

			shaderToRender->BindShader();
			instanceBuffer.bind(1u);
			meshData.bind(2u, shaderTypes::VS);
			materialData.bind(2u, shaderTypes::PS);

			uint32_t renderedInstances = 0;
			for (const auto& perModel : perModel) {
				if (perModel.model.get() == nullptr) continue;
				perModel.model->m_vertices.bind();
				perModel.model->m_indices.bind();
				for (uint32_t meshIndex = 0; meshIndex < perModel.perMesh.size(); ++meshIndex) {
					const Mesh& mesh = perModel.model->m_meshes[meshIndex];
					const auto& meshRange = perModel.model->m_ranges[meshIndex];
					meshData.updateBuffer(reinterpret_cast<const MeshData*>(mesh.instances.data())); // ... update shader local per-mesh uniform buffer
					for (const auto& perMaterial : perModel.perMesh[meshIndex].perMaterial) {
						if (perMaterial.instances.empty()) continue;
						const auto& material = perMaterial.material;

						MaterialData data = { vec4((float)material.opaqueTextures.usedTextures, material.opaqueTextures.roughness, material.opaqueTextures.metalness,0.0f) };

						materialData.updateBuffer(&data);
						uint32_t numInstances = uint32_t(perMaterial.instances.size());
						// Custom rendering logic for TextureMaterial

						perMaterial.material.opaqueTextures.albedoTexture->BindTexture(2u);
						perMaterial.material.opaqueTextures.roughnessTexture->BindTexture(3u);
						perMaterial.material.opaqueTextures.metalnessTexture->BindTexture(4u);
						perMaterial.material.opaqueTextures.normalTexture->BindTexture(5u);
						perMaterial.material.noiseTexture->BindTexture(14u);

						d3d->GetContext()->DrawIndexedInstanced(meshRange.indexNum, numInstances, meshRange.indexOffset, meshRange.vertexOffset, renderedInstances);
						renderedInstances += numInstances;
					}
				}
			}
		}

	};

	template <>
	inline void OpaqueInstances<Instances::IncinerationInstance, Materials::DissolutionMaterial>::update(float time) {

		std::list<uint32_t> ids;

		for (size_t i = 0; i < perModel.size(); i++)
		{
			float modelBoxSize = perModel[i].model->box.size();
			vec3 modelBoxMin = perModel[i].model->box.min;
			vec3 modelBoxMax = perModel[i].model->box.max;
			for (uint32_t meshIndex = 0; meshIndex < perModel[i].perMesh.size(); ++meshIndex)
			{
				const Mesh& mesh = perModel[i].model->m_meshes[meshIndex];

				for (size_t j = 0; j < perModel[i].perMesh[meshIndex].perMaterial.size(); j++)
				{
					auto& instances = perModel[i].perMesh[meshIndex].perMaterial[j].instances;

					uint32_t numModelInstances = (uint32_t)instances.size();

					if (std::find(ids.begin(), ids.end(), instances[0].transformsId) != ids.end())
						continue;

					for (uint32_t index = 0; index < numModelInstances; ++index)
					{
						auto& transform = TransformSystem::Init()->GetModelTransforms(instances[index].transformsId)[meshIndex].modelToWold;

						vec3 transformedMin = vec4(modelBoxMin, 1.0f) * transform;
						vec3 transformedMax = vec4(modelBoxMax, 1.0f) * transform;

						if (instances[index].instanceData.sphereRadius * instances[index].instanceData.sphereRadius > (transformedMax - transformedMin).length_squared())
						{
							ids.push_back(instances[index].transformsId);
							continue;
						}

						instances[index].instanceData.previousRadius = instances[index].instanceData.sphereRadius;
						instances[index].instanceData.sphereRadius += modelBoxSize * time * 0.3f;
					}
				}
			}
		}

		for (auto id : ids)
		{
			removeByTransformId(id, true);
			DecalSystem::Init()->DeleteDecalByTransformId(id);
		}

		updateInstanceBuffers();
	}

	template <>
	inline void OpaqueInstances<Instances::IncinerationInstance, Materials::DissolutionMaterial>::renderUsingShader(std::shared_ptr<shader> shaderToRender)
	{
		// Custom render implementation for TextureMaterial
		if (instanceBuffer.getSize() == 0)
			return;

		D3D* d3d = D3D::GetInstance();

		shaderToRender->BindShader();
		instanceBuffer.bind(1u);
		meshData.bind(2u, shaderTypes::VS);
		materialData.bind(2u, shaderTypes::PS);
		materialData.bind(4u, shaderTypes::GS);

		uint32_t renderedInstances = 0;
		for (const auto& perModel : perModel) {
			if (perModel.model.get() == nullptr) continue;
			perModel.model->m_vertices.bind();
			perModel.model->m_indices.bind();
			for (uint32_t meshIndex = 0; meshIndex < perModel.perMesh.size(); ++meshIndex) {
				const Mesh& mesh = perModel.model->m_meshes[meshIndex];
				const auto& meshRange = perModel.model->m_ranges[meshIndex];
				meshData.updateBuffer(reinterpret_cast<const MeshData*>(mesh.instances.data())); // ... update shader local per-mesh uniform buffer
				for (const auto& perMaterial : perModel.perMesh[meshIndex].perMaterial) {
					if (perMaterial.instances.empty()) continue;
					const auto& material = perMaterial.material;

					MaterialData data = { vec4((float)material.opaqueTextures.usedTextures, material.opaqueTextures.roughness, material.opaqueTextures.metalness, (float)perModel.model->m_indices.getSize()) };

					materialData.updateBuffer(&data);
					uint32_t numInstances = uint32_t(perMaterial.instances.size());
					// Custom rendering logic for TextureMaterial

					perMaterial.material.opaqueTextures.albedoTexture->BindTexture(2u);
					perMaterial.material.opaqueTextures.roughnessTexture->BindTexture(3u);
					perMaterial.material.opaqueTextures.metalnessTexture->BindTexture(4u);
					perMaterial.material.opaqueTextures.normalTexture->BindTexture(5u);
					perMaterial.material.noiseTexture->BindTexture(14u);

					d3d->GetContext()->DrawIndexedInstanced(meshRange.indexNum, numInstances, meshRange.indexOffset, meshRange.vertexOffset, renderedInstances);
					renderedInstances += numInstances;
				}
			}
		}
	}
}

