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
	int sixth = incinerationGroup.intersect(r, hInfo);

	if (sixth != -1)
		return sixth;
	if (fifth != -1)
		return fifth;
	if (fourth != -1)
		return fourth;
	return third != -1? third: (second != -1? second: first);
}

int Engine::MeshSystem::intersectMesh(const ray& r, hitInfo& hInfo)
{
	int first = hologramGroup.intersectMesh(r, hInfo);
	int second = normVisGroup.intersectMesh(r, hInfo);
	int third = opaqueGroup.intersectMesh(r, hInfo);
	int fourth = emmisiveGroup.intersectMesh(r, hInfo);
	int fifth = dissolutionGroup.intersectMesh(r, hInfo);

	if (fifth != -1)
		return fifth;
	if (fourth != -1)
		return fourth;
	return third != -1 ? third : (second != -1 ? second : first);
}

int Engine::MeshSystem::getTransformIdByMeshInstanceId(uint32_t meshInstanceId)
{
	int first = hologramGroup.getTrasnformIdByInstanceMeshId(meshInstanceId);
	int second = normVisGroup.getTrasnformIdByInstanceMeshId(meshInstanceId);
	int third = opaqueGroup.getTrasnformIdByInstanceMeshId(meshInstanceId);
	int fourth = emmisiveGroup.getTrasnformIdByInstanceMeshId(meshInstanceId);
	int fifth = dissolutionGroup.getTrasnformIdByInstanceMeshId(meshInstanceId);

	if (fifth != -1)
		return fifth;
	if (fourth != -1)
		return fourth;
	return third != -1 ? third : (second != -1 ? second : first);
}

void Engine::MeshSystem::updateInstanceBuffers()
{
	normVisGroup.updateInstanceBuffers();
	hologramGroup.updateInstanceBuffers();
	opaqueGroup.updateInstanceBuffers();
	emmisiveGroup.updateInstanceBuffers();
	dissolutionGroup.updateInstanceBuffers();
	incinerationGroup.updateInstanceBuffers();
	boneWeightShow.updateInstanceBuffers();
}

void Engine::MeshSystem::render()
{
	opaqueGroup.render();
	normVisGroup.render();
	hologramGroup.render();
	emmisiveGroup.render();
	boneWeightShow.render();
}

void Engine::MeshSystem::renderGBuffer(ID3D11DepthStencilState* depthStencilState)
{
	auto context = D3D::GetInstance()->GetContext();

	context->OMSetDepthStencilState(depthStencilState, 2u);
	emmisiveGroup.renderUsingShader(emmisiveGroup.GBufferShader);
	hologramGroup.renderUsingShader(hologramGroup.GBufferShader);

	context->OMSetDepthStencilState(depthStencilState, 1u);
	opaqueGroup.renderUsingShader(opaqueGroup.GBufferShader);
	dissolutionGroup.renderUsingShader(dissolutionGroup.GBufferShader);
	incinerationGroup.renderUsingShader(incinerationGroup.GBufferShader);
}

void Engine::MeshSystem::defferedRender(ID3D11DepthStencilState* dsStencilOnlyState)
{
	auto context = D3D::GetInstance()->GetContext();

	context->OMSetDepthStencilState(dsStencilOnlyState, 1u);
	litDefferedShader->BindShader();
	context->Draw(3u, 0);
	context->OMSetDepthStencilState(dsStencilOnlyState, 2u);
	emissiveDefferedShader->BindShader();
	context->Draw(3u, 0);
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
	auto inceShadow = ShaderManager::GetShader("IncinerationPLShadow");
	ShadowSystem::Init()->RenderPointLightShadowMaps(lightPositions, { ShadowSystem::ShadowRenderGroupInfo{RenderGroups::OPAQUEGROUP,opaqueShadowShader},
		ShadowSystem::ShadowRenderGroupInfo{RenderGroups::DISSOLUTION,dissShadowShader}, ShadowSystem::ShadowRenderGroupInfo{ RenderGroups::INCINERATION, inceShadow } });
}

void Engine::MeshSystem::renderDepth2D(const std::vector<Engine::SpotLight>& spotlights)
{
	ShadowSystem::Init()->PrecomputeSpotProjections(spotlights);

	auto opaqueShadowShader = ShaderManager::GetShader("shadow");
	auto dissShadowShader = ShaderManager::GetShader("DissShadow");
	auto inceShadow = ShaderManager::GetShader("IncinerationShadow");
	ShadowSystem::Init()->RenderSpotLightShadowMaps({ ShadowSystem::ShadowRenderGroupInfo{RenderGroups::OPAQUEGROUP,opaqueShadowShader},
		ShadowSystem::ShadowRenderGroupInfo{RenderGroups::DISSOLUTION,dissShadowShader}, ShadowSystem::ShadowRenderGroupInfo{ RenderGroups::INCINERATION, inceShadow } });
}

void Engine::MeshSystem::renderDepth2DDirectional(const std::vector<DirectionalLight>& directionalLights, const Camera* camera)
{
	ShadowSystem::Init()->PrecomputeDirectionalProjections(directionalLights, camera);

	auto opaqueShadowShader = ShaderManager::GetShader("shadow");
	auto dissShadowShader = ShaderManager::GetShader("DissShadow");
	auto inceShadow = ShaderManager::GetShader("IncinerationShadow");
	ShadowSystem::Init()->RenderDirectLightShadowMaps({ ShadowSystem::ShadowRenderGroupInfo{RenderGroups::OPAQUEGROUP,opaqueShadowShader},
		ShadowSystem::ShadowRenderGroupInfo{RenderGroups::DISSOLUTION,dissShadowShader}, ShadowSystem::ShadowRenderGroupInfo{ RenderGroups::INCINERATION, inceShadow } });
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

void Engine::MeshSystem::setLitDefferedShader(std::shared_ptr<shader> lit)
{
	litDefferedShader = lit;
}

void Engine::MeshSystem::setEmissiveDefferedShader(std::shared_ptr<shader> emissive)
{
	emissiveDefferedShader = emissive;
}