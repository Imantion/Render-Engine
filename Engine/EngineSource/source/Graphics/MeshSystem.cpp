#include "Graphics/MeshSystem.h"



std::mutex Engine::MeshSystem::mutex_;
Engine::MeshSystem* Engine::MeshSystem::pInstance = nullptr;

int Engine::MeshSystem::intersect(const ray& r, hitInfo& hInfo)
{
	int first = hologramGroup.intersect(r, hInfo);
	int second = normVisGroup.intersect(r, hInfo);
	int third = opaqueGroup.intersect(r, hInfo);
	int fourth = emmisiveGroup.intersect(r, hInfo);


	if (fourth != -1)
		return fourth;
	return third != -1? third: (second != -1? second: first);
}

void Engine::MeshSystem::updateInstanceBuffers()
{
	normVisGroup.updateInstanceBuffers();
	hologramGroup.updateInstanceBuffers();
	opaqueGroup.updateInstanceBuffers();
	emmisiveGroup.updateInstanceBuffers();
}

void Engine::MeshSystem::render()
{
	normVisGroup.render();
	hologramGroup.render();
	opaqueGroup.render();
	emmisiveGroup.render();
}

Engine::MeshSystem* Engine::MeshSystem::Init()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (pInstance == nullptr)
	{
		pInstance = new MeshSystem();
	}

	return pInstance;
}

void Engine::MeshSystem::Deinit()
{
	delete pInstance;
	pInstance = nullptr;
}

template <>
inline void Engine::OpaqueInstances<Engine::MeshSystem::PBRInstance, Materials::OpaqueTextureMaterial>::render()
{

	// Custom render implementation for TextureMaterial
	if (instanceBuffer.getSize() == 0)
		return;

	D3D* d3d = D3D::GetInstance();
	for (size_t i = 0; i < m_shaders.size(); i++) {
		if (!m_shaders[i]->isEnabled)
			continue;
		m_shaders[i]->BindShader();
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

					MaterialData data = { vec4(material.usedTextures, material.roughness, material.metalness,0.0f)};

					materialData.updateBuffer(&data);
					uint32_t numInstances = uint32_t(perMaterial.instances.size());
					// Custom rendering logic for TextureMaterial

					perMaterial.material.albedoTexture->BindTexture(2u);
					perMaterial.material.roughnessTexture->BindTexture(3u);
					perMaterial.material.metalnessTexture->BindTexture(4u);
					perMaterial.material.normalTexture->BindTexture(5u);

					d3d->GetContext()->DrawIndexedInstanced(meshRange.indexNum, numInstances, meshRange.indexOffset, meshRange.vertexOffset, renderedInstances);
					renderedInstances += numInstances;
				}
			}
		}
	}
}