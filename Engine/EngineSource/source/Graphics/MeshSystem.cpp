#include "Graphics/MeshSystem.h"
#include "Graphics/LightSystem.h"
#include "Graphics/ReflectionCapture.h"
#include "Graphics/ShadowSystem.h"
#include "Render/Camera.h"

std::mutex Engine::MeshSystem::mutex_;
Engine::MeshSystem* Engine::MeshSystem::pInstance = nullptr;

int Engine::MeshSystem::intersect(const ray& r, hitInfo& hInfo)
{
	int first = hologramGroup.intersect(r, hInfo);
	int second = normVisGroup.intersect(r, hInfo);
	int third = opaqueGroup.intersect(r, hInfo);
	int fourth = emmisiveGroup.intersect(r, hInfo);
	int fifth = dissolutionGroup.intersect(r, hInfo);

	if (fifth != -1)
		return fifth;
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
	shadowGroup.updateInstanceBuffers();
	dissolutionGroup.updateInstanceBuffers();
}

void Engine::MeshSystem::render()
{
	auto context = D3D::GetInstance()->GetContext();
	ID3D11DepthStencilState* dsState;
	UINT stencilValue;
	context->OMGetDepthStencilState(&dsState, &stencilValue);

	context->OMSetDepthStencilState(dsState, 1u);
	opaqueGroup.render();

	context->OMSetDepthStencilState(dsState, 2u);
	normVisGroup.render();
	hologramGroup.render();
	emmisiveGroup.render();

	context->OMSetDepthStencilState(dsState, stencilValue);
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

void Engine::MeshSystem::renderTranslucent()
{
	dissolutionGroup.render();
}

void Engine::MeshSystem::renderDepthCubemaps(const std::vector<vec3>& lightPositions)
{
	auto opaqueShadowShader = ShaderManager::GetShader("PLshadow");
	auto dissShadowShader = ShaderManager::GetShader("DissPLshadow");

	ShadowSystem::Init()->RenderPointLightShadowMaps(lightPositions, { ShadowSystem::ShadowRenderGroupInfo{RenderGroups::OPAQUEGROUP,opaqueShadowShader},
		ShadowSystem::ShadowRenderGroupInfo{RenderGroups::DISSOLUTION,dissShadowShader} });
}

void Engine::MeshSystem::renderDepth2D(const std::vector<Engine::SpotLight>& spotlights)
{
	ShadowSystem::Init()->PrecomputeSpotProjections(spotlights);

	auto opaqueShadowShader = ShaderManager::GetShader("shadow");
	auto dissShadowShader = ShaderManager::GetShader("DissShadow");
	ShadowSystem::Init()->RenderSpotLightShadowMaps({ ShadowSystem::ShadowRenderGroupInfo{RenderGroups::OPAQUEGROUP,opaqueShadowShader},
		ShadowSystem::ShadowRenderGroupInfo{RenderGroups::DISSOLUTION,dissShadowShader} });
}

void Engine::MeshSystem::renderDepth2DDirectional(const std::vector<DirectionalLight>& directionalLights, const Camera* camera)
{
	ShadowSystem::Init()->PrecomputeDirectionalProjections(directionalLights, camera);

	auto opaqueShadowShader = ShaderManager::GetShader("shadow");
	auto dissShadowShader = ShaderManager::GetShader("DissShadow");
	ShadowSystem::Init()->RenderDirectLightShadowMaps({ ShadowSystem::ShadowRenderGroupInfo{RenderGroups::OPAQUEGROUP,opaqueShadowShader},
		ShadowSystem::ShadowRenderGroupInfo{RenderGroups::DISSOLUTION,dissShadowShader} });
}


template <>
inline void Engine::OpaqueInstances<Instances::PBRInstance, Materials::OpaqueTextureMaterial>::renderUsingShader(std::shared_ptr<shader> shaderToRender)
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

				MaterialData data = { vec4((float)material.usedTextures, material.roughness, material.metalness,0.0f)};

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