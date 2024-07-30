#include "Graphics/MeshSystem.h"
#include "Graphics/LightSystem.h"
#include "Graphics/ReflectionCapture.h"

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
	shadowGroup.updateInstanceBuffers();
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

Engine::MeshSystem::MeshSystem()
{
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
	sampDesc.BorderColor[0] = 1.0f;
	sampDesc.BorderColor[1] = 1.0f;
	sampDesc.BorderColor[2] = 1.0f;
	sampDesc.BorderColor[3] = 1.0f;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT hr = D3D::GetInstance()->GetDevice()->CreateSamplerState(&sampDesc, &compsampler);
	assert(SUCCEEDED(hr));

	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.DepthBias = 0.0f;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.AntialiasedLineEnable = false;

	hr = D3D::GetInstance()->GetDevice()->CreateRasterizerState(&rasterDesc, &pRasterizerState);
	assert(SUCCEEDED(hr));

	pointLightCB.create(D3D11_USAGE_DYNAMIC, MAX_POINT_LIGHTS);
}

void Engine::MeshSystem::createDepthCubemaps(size_t amount)
{
	dsvs.clear();
	dsvs.resize(amount);

	auto device = D3D::GetInstance()->GetDevice();

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = 1024;
	textureDesc.Height = 1024;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = amount * 6;
	textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &texture);

	assert(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	depthStencilViewDesc.Texture2DArray.ArraySize = 6;
	depthStencilViewDesc.Texture2DArray.MipSlice = 0;

	for (size_t i = 0; i < amount; i++)
	{
		depthStencilViewDesc.Texture2DArray.FirstArraySlice = i * 6;
		hr = device->CreateDepthStencilView(texture.Get(), &depthStencilViewDesc, &dsvs[i]);
		assert(SUCCEEDED(hr));
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));

	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.ArraySize = amount * 6;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, &srvPointLigts);
	assert(SUCCEEDED(hr));
}

void Engine::MeshSystem::bindShadowMapsData(UINT pointLightShadowTexturesSlot, UINT pointLightCBslot)
{
	auto context = D3D::GetInstance()->GetContext();
	context->PSSetShaderResources(pointLightShadowTexturesSlot, 1, srvPointLigts.GetAddressOf());
	pointLightCB.bind(pointLightCBslot, VS);
}

void Engine::MeshSystem::renderDepthCubemaps(const std::vector<vec3>& lightPositions)
{
	if (dsvs.size() != lightPositions.size())
	{
		createDepthCubemaps(lightPositions.size());
		cbViewProjdata.resize(lightPositions.size());
	}
		

	auto context = D3D::GetInstance()->GetContext();

	ID3D11ShaderResourceView* const pSRV[1] = { NULL };
	context->PSSetShaderResources(11, 1u, pSRV);

	float farPlane = 100.0f;
	mat4 projection = projectionMatrix((float)M_PI_2, 0.01f, farPlane, 1.0f);

	struct lightInfo
	{
		vec3 position;
		float radius;
	} lightBufferData;

	ConstBuffer<cubeViewProjections> cbProjections;
	cbProjections.create();

	ConstBuffer<lightInfo> cbLightInfo;
	cbLightInfo.create();

	ConstBuffer<vec4> psConstBuffer;
	psConstBuffer.create();

	D3D11_VIEWPORT viewPort = {};
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	viewPort.Width = 1024;
	viewPort.Height = 1024;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;

	context->RSSetViewports(1, &viewPort);
	context->PSSetSamplers(5u, 1u, compsampler.GetAddressOf());
	/*context->RSSetState(pRasterizerState.Get());*/

	for (size_t i = 0; i < lightPositions.size(); i++)
	{
		context->ClearDepthStencilView(dsvs[i].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0u);
		cbViewProjdata[i] = {
			viewMatrix(lightPositions[i], vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)) * projection,
			viewMatrix(lightPositions[i], vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)) * projection,
			viewMatrix(lightPositions[i], vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)) * projection,
			viewMatrix(lightPositions[i], vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)) * projection,
			viewMatrix(lightPositions[i], vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f)) * projection,
			viewMatrix(lightPositions[i], vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 1.0f)) * projection,
		};

		vec4 psData(lightPositions[i], farPlane);

		cbProjections.updateBuffer(&cbViewProjdata[i]);
		cbProjections.bind(0u, GS);

		psConstBuffer.updateBuffer(&psData);
		psConstBuffer.bind(10u, PS);

		context->OMSetRenderTargets(0u, nullptr, dsvs[i].Get());

		shadowGroup.render();
	}

	pointLightCB.updateBuffer(cbViewProjdata.data(), cbViewProjdata.size());

	context->OMSetRenderTargets(0u, nullptr, nullptr);
	context->RSSetState(nullptr);
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
}