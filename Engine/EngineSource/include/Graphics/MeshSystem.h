#pragma once
#include <vector>
#include <memory>
#include "Math/matrix.h"
#include "Math/math.h"
#include "Math/hitable.h"
#include "Graphics/Buffers.h"
#include "Graphics/Model.h"
#include "Graphics/ShaderManager.h"

namespace Engine
{
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

		struct MeshData
		{
			mat4 meshToModel;
		};

		struct MaterialData
		{
			M material;
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
		ConstBuffer<MaterialData> materialData;
	public:

		void updateShader(std::shared_ptr<shader> shdr)
		{
			m_shader = shdr;
		}

		OpaqueInstances() { meshData.create(); materialData.create(); }

		instanceOfModel intersect(const ray& r, hitInfo& hInfo)
		{
			instanceOfModel inst;
			inst.modelIndex = -1;
			inst.perMaterialIndex = -1;
			inst.materialIndex = -1;

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
							transformedRay.origin = vec4(r.origin, 1.0f) * mat4::Inverse(instances[index].tranformation) * mesh.invInstances[0];
							transformedRay.direction = vec4(r.direction, 0.0f) * mesh.invInstances[0];
							
							if (mesh.intersect(transformedRay, hInfo))
							{
								inst.modelIndex = i;
								inst.perMaterialIndex = j;
								inst.materialIndex = index;
								hInfo.p = r.point_at_parameter(hInfo.t);
							}
						}
					}
				}
			}

			return inst;
		}

		void getInstanceTransform(int modelIndex, int perMaterialIndex,int materalIndex, std::vector<mat4*>& transforms)
		{
			if (modelIndex < 0 || modelIndex > perModel.size())
				return;

			transforms.reserve(perModel[modelIndex].model->m_meshes.size());

			for (size_t i = 0; i < perModel[modelIndex].model->m_meshes.size(); i++)
			{
				auto& instances = perModel[modelIndex].perMesh[i].perMaterial[perMaterialIndex].instances;
					transforms.push_back(&instances[materalIndex].tranformation);
			}
		}


		void addModel(std::shared_ptr<Model> model, const M& material, const vec3& position) // rotation order means!
		{
			//float pi = 3.14159265359f;
			//auto rotX = mat4::rotateX(pi * (-xRotation) / 360.0f);
			//auto rotY = mat4::rotateY(pi * (-yRotation) / 360.0f);
			//auto rotZ = mat4::rotateZ(pi * (-zRotation) / 360.0f);

			I transform{ transformMatrix(position, vec3(0.0f, 0.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)) };
			auto it = perModel.end();
			for (auto i = perModel.begin(); i != perModel.end(); i++)
			{
				if (i->model.get() == model.get())
					it = i;
			}

			if (it == perModel.end())
			{
				std::vector<I> inst(1, transform);

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
							perMaterial.instances.push_back(transform);
							inserted = true;
						}
					}

					if (!inserted)
					{
						std::vector<I> inst;
						inst.push_back(transform);
						pModel->perMesh[meshIndex].perMaterial.push_back(PerMaterial{ material, inst });
					}
				}
			}

		}

		void updateInstanceBufferData(int index, const I* instance)
		{
			D3D11_MAPPED_SUBRESOURCE mapping;
			instanceBuffer.map(mapping);
			I* dst = static_cast<I*>(mapping.pData);

			dst[index] = instance;
			instanceBuffer.unmap();
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
			d3d->GetContext()->PSSetConstantBuffers(3u, 1, meshData.m_constBuffer.GetAddressOf());
			d3d->GetContext()->PSSetConstantBuffers(2u, 1, materialData.m_constBuffer.GetAddressOf());

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
			vec3 shortWaveColor;
			float padding0;
			vec3 longWaveColor;
			float padding1;

			bool operator==(const Material& other) const
			{
				return shortWaveColor == other.shortWaveColor && longWaveColor == other.longWaveColor;
			}
		};

		struct Instance // all other template instances must inherit this one
		{
			mat4 tranformation;
		};

		OpaqueInstances<Instance, Material> hologramGroup;
		OpaqueInstances<Instance, Material> normVisGroup;

		std::vector<mat4*> intersect(const ray& r, hitInfo& hInfo);

		void updateInstanceBuffers();
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