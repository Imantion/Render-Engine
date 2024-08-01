#include "Graphics/MeshSystem.h"
#include "Graphics/LightSystem.h"
#include "Graphics/ReflectionCapture.h"
#include "Graphics/ShadowManager.h"

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
	rasterDesc.DepthBias = -128;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = -1.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.AntialiasedLineEnable = false;

	hr = D3D::GetInstance()->GetDevice()->CreateRasterizerState(&rasterDesc, &pRasterizerState);
	assert(SUCCEEDED(hr));
}

void Engine::MeshSystem::createDepthCubemaps(size_t amount)
{
	plDSVS.clear();
	plDSVS.resize(amount);

	auto device = D3D::GetInstance()->GetDevice();

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = 1024;
	textureDesc.Height = 1024;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = (UINT)amount * 6u;
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
		depthStencilViewDesc.Texture2DArray.FirstArraySlice = (UINT)i * 6u;
		hr = device->CreateDepthStencilView(texture.Get(), &depthStencilViewDesc, &plDSVS[i]);
		assert(SUCCEEDED(hr));
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));

	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBEARRAY;
	srvDesc.TextureCubeArray.MipLevels = 1;
	srvDesc.TextureCubeArray.First2DArrayFace = 0;
	srvDesc.TextureCubeArray.MostDetailedMip = 0;
	srvDesc.TextureCubeArray.NumCubes = (UINT)amount;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, &srvPointLigts);
	assert(SUCCEEDED(hr));
}

void Engine::MeshSystem::createDepth2DSpotLightsMap(size_t amount)
{
	slDSVS.resize(amount);

	auto device = D3D::GetInstance()->GetDevice();

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = 1024;
	textureDesc.Height = 1024;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = (UINT)amount;
	textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;

	HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &texture);
	assert(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	depthStencilViewDesc.Texture2DArray.ArraySize = 1u;
	depthStencilViewDesc.Texture2DArray.MipSlice = (UINT)0;

	for (size_t i = 0; i < amount; i++)
	{
		depthStencilViewDesc.Texture2DArray.FirstArraySlice = (UINT)0;
		hr = device->CreateDepthStencilView(texture.Get(), &depthStencilViewDesc, slDSVS[i].GetAddressOf());
		assert(SUCCEEDED(hr));
	}
	
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));

	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.ArraySize = (UINT)amount;
	srvDesc.Texture2DArray.FirstArraySlice = 0u;
	srvDesc.Texture2DArray.MipLevels = 1u;
	srvDesc.Texture2DArray.MostDetailedMip = 0u;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, &srvSpotLigts);
	assert(SUCCEEDED(hr));
}

void Engine::MeshSystem::bindShadowMapsData(UINT pointLightShadowTexturesSlot, UINT compSamplerSlot)
{
	auto context = D3D::GetInstance()->GetContext();
	context->PSSetSamplers(compSamplerSlot, 1, compsampler.GetAddressOf());
	context->PSSetShaderResources(pointLightShadowTexturesSlot, 1, srvPointLigts.GetAddressOf());
	context->PSSetShaderResources(12, 1, srvSpotLigts.GetAddressOf());
}

void Engine::MeshSystem::renderDepthCubemaps(const std::vector<vec3>& lightPositions)
{
	ShadowManager::Init()->RenderPointLightShadowMaps(lightPositions, opaqueGroup);
}

void Engine::MeshSystem::renderDepth2D(const std::vector<Engine::SpotLight>& spotlights)
{
	if (spotlights.size() != slDSVS.size())
		createDepth2DSpotLightsMap(spotlights.size());

	auto shader = ShaderManager::GetShader("shadowShader2");
	shader->EnableShader();

	auto context = D3D::GetInstance()->GetContext();

	float farPlane = 100.0f;

	struct lightViewProjections
	{
		mat4 lightViewProjection;
	};
	
	ConstBuffer<lightViewProjections> cbProjections;
	cbProjections.create();

	D3D11_VIEWPORT boundedViewport;
	UINT numViewports = 1u;
	context->RSGetViewports(&numViewports, &boundedViewport);

	D3D11_VIEWPORT viewPort = {};
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	viewPort.Width = 1024;
	viewPort.Height = 1024;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;

	context->RSSetViewports(1, &viewPort);
	context->RSSetState(pRasterizerState.Get());


	auto TS = TransformSystem::Init();
	for (size_t i = 0; i < spotlights.size(); i++)
	{
		vec3 position = spotlights[i].position;
		if(spotlights[i].bindedObjectId != -1)
			position += (vec3&)*TS->GetModelTransforms(spotlights[i].bindedObjectId)[0].modelToWold[3];
		mat4 projection = projectionMatrix(spotlights[i].cutoffAngle * 2.0f, 0.01f, farPlane, 1.0f);

		lightViewProjections constantBufferData{
			 LightSystem::Init()->getFlashLightViewProjection(),
		};

		cbProjections.updateBuffer(&constantBufferData);
		cbProjections.bind(4u, VS);


		context->ClearDepthStencilView(slDSVS[i].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0u);
		context->OMSetRenderTargets(0u, nullptr, slDSVS[i].Get());

		shadowGroup.render();
	}

	context->OMSetRenderTargets(0u, nullptr, nullptr);
	context->RSSetState(nullptr);
	context->RSSetViewports(1u, &boundedViewport);

	shader->DisableShader();
}


template <>
inline void Engine::OpaqueInstances<Engine::MeshSystem::PBRInstance, Materials::OpaqueTextureMaterial>::renderUsingShader(std::shared_ptr<shader> shaderToRender)
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