#include "Graphics/ShadowSystem.h"
#include "Math/math.h"
#include "Math/matrix.h"
#include "Graphics/TransformSystem.h"
#include "Graphics/MeshSystem.h"

Engine::ShadowSystem* Engine::ShadowSystem::m_instance;
std::mutex Engine::ShadowSystem::m_mutex;

Engine::ShadowSystem* Engine::ShadowSystem::Init()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (m_instance == nullptr)
	{
		m_instance = new ShadowSystem();
	}

	return m_instance;
}

void Engine::ShadowSystem::Deinit()
{
	delete m_instance;
	m_instance = nullptr;
}

void Engine::ShadowSystem::SetShadowShaders(std::shared_ptr<shader> pointLight, std::shared_ptr<shader> spotLight, std::shared_ptr<shader> directionalLight)
{
	m_plShader = pointLight;
	m_slShader = spotLight;
	m_dlShader = directionalLight;
}

void Engine::ShadowSystem::SetShadowTextureResolution(UINT resolution)
{
	m_shadowResolution = resolution;
	m_viewport.Width = (FLOAT)resolution;
	m_viewport.Height = (FLOAT)resolution;
	
	createPointLightShadowMaps(m_plDSVS.size());
	Create2DShadowMaps(m_slDSVS.size(), m_slDSVS, m_srvSpotLigts);
	Create2DShadowMaps(m_dlDSVS.size(), m_dlDSVS, m_srvDirLigts);
}

UINT Engine::ShadowSystem::GetShadowTextureResolution()
{
	return m_shadowResolution;
}

void Engine::ShadowSystem::SetProjectionInfo(float nearPlane, float farPlane)
{
	m_ProjectionInfo.nearPlane = nearPlane;
	m_ProjectionInfo.farPlane = farPlane;
}

void Engine::ShadowSystem::PrecomputeDirectionalProjections(const std::vector<DirectionalLight>& lights, const Camera* camera)
{
	if (lights.size() != m_dlDSVS.size())
	{
		m_dlViewProjections.resize(lights.size());
		Create2DShadowMaps(lights.size(), m_dlDSVS, m_srvDirLigts);
	}


	vec3 frustrumCorners[8] = {
	  vec3(-1.0f, 1.0f, 0.0f),
	  vec3(1.0f,  1.0f, 0.0f),
	  vec3(-1.0f,-1.0f, 0.0f),
	  vec3(1.0f, -1.0f, 0.0f),
	  vec3(-1.0f, 1.0f, 1.0f),
	  vec3(1.0f, 1.0f, 1.0f),
	  vec3(-1.0f, -1.0f, 1.0f),
	  vec3(1.0f, -1.0f, 1.0f)
	};

	vec3 frustrumCentr;

	for (size_t i = 0; i < 8; i++)
	{
		vec4 viewSpace = vec4(frustrumCorners[i], 1.0f) * camera->getInverseProjectionMatrix();
		frustrumCorners[i] = (viewSpace / viewSpace.w) * camera->getInverseViewMatrix();

		frustrumCentr += frustrumCorners[i];
	}

	frustrumCentr = frustrumCentr / 8.0f;


	float radius = (frustrumCorners[2] - frustrumCorners[5]).length() * 0.5f;

	float texelSize = (float)m_shadowResolution / (2.0f * radius);

	mat4 scalarMatrix(vec4(texelSize, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, texelSize, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, texelSize, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 1.0f));


	mat4 ortProjection = orthographicProjecton(radius, -radius, radius, -radius, -radius, radius);

	vec3 zero(0.0f);
	vec3 up(0.0f, 1.0f, 0.0f);
	for (size_t i = 0; i < lights.size(); i++)
	{
		mat4 scalaredView = viewMatrix(zero, -lights[i].direction, up) * scalarMatrix;
		mat4 inversedScalaerView = mat4::Inverse(scalaredView);

		vec3 newFrustrumCentr = vec4(frustrumCentr, 1.0f) * scalaredView;
		newFrustrumCentr.x = floor(newFrustrumCentr.x);
		newFrustrumCentr.y = floor(newFrustrumCentr.y);

		newFrustrumCentr = vec4(newFrustrumCentr, 1.0f) * inversedScalaerView;

		vec3 eye = newFrustrumCentr - (lights[i].direction * radius);
		mat4 view = LookAt(eye, newFrustrumCentr, up);

		float maxZ = std::numeric_limits<float>::min();
		float minZ = std::numeric_limits<float>::max();

		for (size_t i = 0; i < 8; i++)
		{
			vec4 viewCorner = vec4(frustrumCorners[i], 1.0f) * view;
			maxZ = Max(maxZ, viewCorner.z);
			minZ = Min(minZ, viewCorner.z);
		}

		float z = maxZ - minZ + 10.0f;

		ortProjection[2][2] = 1.0f / (-z - z);

		m_dlViewProjections[i] = view * ortProjection;
	}

	m_dlConstBuff.updateBuffer(m_dlViewProjections.data(), (UINT)m_dlViewProjections.size());
}

void Engine::ShadowSystem::PrecomputeSpotProjections(const std::vector<SpotLight>& lights)
{
	if (lights.size() != m_slDSVS.size())
	{
		m_slViewProjections.resize(lights.size());
		Create2DShadowMaps(lights.size(), m_slDSVS, m_srvSpotLigts);
	}

	auto TS = TransformSystem::Init();

	for (size_t i = 0; i < lights.size(); i++)
	{
		vec3 position = lights[i].position;

		if (lights[i].bindedObjectId != -1)
		{
			auto& transform = TS->GetModelTransforms(lights[i].bindedObjectId)[0].modelToWold;
			position += (vec3&)*transform[3];
			vec3 direction = vec4(lights[i].direction, 0.0f) * transform;

			mat4 projection = projectionMatrix(lights[i].cutoffAngle * 2.0f, m_ProjectionInfo.nearPlane, m_ProjectionInfo.farPlane, 1.0f);
			m_slViewProjections[i] = viewMatrix(position, direction, topFromDir(direction)) * projection;
		}
	}
}

void Engine::ShadowSystem::RenderPointLightShadowMaps(const std::vector<vec3>& lightPositions, const std::vector<ShadowRenderGroupInfo>& renderGroupsInfo)
{
	if (m_plDSVS.size() != lightPositions.size())
		createPointLightShadowMaps(lightPositions.size());

	auto context = D3D::GetInstance()->GetContext();

	mat4 projection = projectionMatrix((float)M_PI_2, m_ProjectionInfo.nearPlane, m_ProjectionInfo.farPlane, 1.0f);

	struct lightViewProjections
	{
		mat4 lightViewProjection[6];
	};

	ConstBuffer<lightViewProjections> cbProjections;
	cbProjections.create();

	ConstBuffer<vec4> psConstBuffer;
	psConstBuffer.create();

	D3D11_VIEWPORT boundedViewport;
	UINT numViewports = 1u;
	context->RSGetViewports(&numViewports, &boundedViewport);


	context->RSSetViewports(1, &m_viewport);

	static const vec3 directions[6] =
	{
		vec3(1.0f, 0.0f, 0.0f), // +X
		vec3(-1.0f, 0.0f, 0.0f), // -X
		vec3(0.0f, 1.0f, 0.0f), // +Y
		vec3(0.0f, -1.0f, 0.0f), // -Y
		vec3(0.0f, 0.0f, 1.0f), // +Z
		vec3(0.0f, 0.0f, -1.0f) // -Z
	};

	for (size_t i = 0; i < lightPositions.size(); i++)
	{
		context->ClearDepthStencilView(m_plDSVS[i].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0u);

		lightViewProjections constantBufferData{
			viewMatrix(lightPositions[i], directions[0], vec3(0.0f, 1.0f, 0.0f)) * projection,
			viewMatrix(lightPositions[i], directions[1], vec3(0.0f, 1.0f, 0.0f)) * projection,
			viewMatrix(lightPositions[i], directions[2], vec3(0.0f, 0.0f, -1.0f)) * projection,
			viewMatrix(lightPositions[i], directions[3], vec3(0.0f, 0.0f, 1.0f)) * projection,
			viewMatrix(lightPositions[i], directions[4], vec3(0.0f, 1.0f, 0.0f)) * projection,
			viewMatrix(lightPositions[i], directions[5], vec3(0.0f, 1.0f, 1.0f)) * projection,
		};

		vec4 psData(lightPositions[i], m_ProjectionInfo.farPlane);

		cbProjections.updateBuffer(&constantBufferData);
		cbProjections.bind(2u, GS);

		psConstBuffer.updateBuffer(&psData);
		psConstBuffer.bind(10u, PS);

		context->OMSetRenderTargets(0u, nullptr, m_plDSVS[i].Get());

		for (size_t i = 0; i < renderGroupsInfo.size(); i++)
		{
			groupToRenderIn(renderGroupsInfo[i].renderFlag, renderGroupsInfo[i].usedShader);
		}
	}

	context->OMSetRenderTargets(0u, nullptr, nullptr);
	context->RSSetViewports(1u, &boundedViewport);
}

void Engine::ShadowSystem::RenderSpotLightShadowMaps(const std::vector<ShadowRenderGroupInfo>& renderGroupsInfo)
{
	auto context = D3D::GetInstance()->GetContext();

	ConstBuffer<mat4> cbProjections;
	cbProjections.create();

	D3D11_VIEWPORT boundedViewport;
	UINT numViewports = 1u;
	context->RSGetViewports(&numViewports, &boundedViewport);

	context->RSSetViewports(1, &m_viewport);

	auto TS = TransformSystem::Init();
	for (size_t i = 0; i < m_slViewProjections.size(); i++)
	{
		cbProjections.updateBuffer(&m_slViewProjections[i]);
		cbProjections.bind(4u, VS);

		context->ClearDepthStencilView(m_slDSVS[i].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0u);
		context->OMSetRenderTargets(0u, nullptr, m_slDSVS[i].Get());

		for (size_t i = 0; i < renderGroupsInfo.size(); i++)
		{
			groupToRenderIn(renderGroupsInfo[i].renderFlag, renderGroupsInfo[i].usedShader);
		}
	}

	m_slConstBuff.updateBuffer(m_slViewProjections.data(), (UINT)m_slViewProjections.size());

	context->OMSetRenderTargets(0u, nullptr, nullptr);
	context->RSSetViewports(1u, &boundedViewport);
}

void Engine::ShadowSystem::RenderDirectLightShadowMaps(const std::vector<ShadowRenderGroupInfo>& renderGroupsInfo)
{
	auto context = D3D::GetInstance()->GetContext();

	ConstBuffer<mat4> cbProjections;
	cbProjections.create();

	D3D11_VIEWPORT boundedViewport;
	UINT numViewports = 1u;
	context->RSGetViewports(&numViewports, &boundedViewport);

	context->RSSetViewports(1, &m_viewport);


	for (size_t i = 0; i < m_dlViewProjections.size(); i++)
	{

		cbProjections.updateBuffer(&m_dlViewProjections[i]);
		cbProjections.bind(4u, VS);

		context->ClearDepthStencilView(m_dlDSVS[i].Get(), D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 0.0f, 0u);
		context->OMSetRenderTargets(0, nullptr, m_dlDSVS[i].Get());

		for (size_t i = 0; i < renderGroupsInfo.size(); i++)
		{
			groupToRenderIn(renderGroupsInfo[i].renderFlag, renderGroupsInfo[i].usedShader);
		}
	}

	context->OMSetRenderTargets(0u, nullptr, nullptr);
	context->RSSetViewports(1u, &boundedViewport);
}

void Engine::ShadowSystem::BindShadowTextures(UINT pointLightTextureSlot , UINT spotLightTextureSlot, UINT directionaLightTexture)
{
	auto context = D3D::GetInstance()->GetContext();
	
	context->PSSetShaderResources(pointLightTextureSlot, 1u, m_srvPointLigts.GetAddressOf());
	context->PSSetShaderResources(spotLightTextureSlot, 1u, m_srvSpotLigts.GetAddressOf());
	context->PSSetShaderResources(directionaLightTexture, 1u, m_srvDirLigts.GetAddressOf());
}

void Engine::ShadowSystem::BindShadowBuffers(UINT spotLightsBufferSlot, UINT directionalLightsBufferSlot)
{
	m_slConstBuff.bind(spotLightsBufferSlot, PS);
	m_dlConstBuff.bind(directionalLightsBufferSlot, PS);
}

void Engine::ShadowSystem::groupToRenderIn(MeshSystem::RenderGroups renderGroupFlags, std::shared_ptr<shader> usedShader)
{
	auto MS = MeshSystem::Init();
	switch (renderGroupFlags)
	{
	case Engine::MeshSystem::HOLOGRAM:
		MS->hologramGroup.renderUsingShader(usedShader);
		break;
	case Engine::MeshSystem::NORMALVIS:
		MS->normVisGroup.renderUsingShader(usedShader);
		break;
	case Engine::MeshSystem::OPAQUEGROUP:
		MS->opaqueGroup.renderUsingShader(usedShader);
		break;
	case Engine::MeshSystem::EMMISIVE:
		MS->emmisiveGroup.renderUsingShader(usedShader);
		break;
	case Engine::MeshSystem::DISSOLUTION:
		MS->dissolutionGroup.renderUsingShader(usedShader);
		break;
	case Engine::MeshSystem::INCINERATION:
		MS->incinerationGroup.renderUsingShader(usedShader);
		break;
	default:
		break;
	}
}

Engine::ShadowSystem::ShadowSystem()
{
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_viewport.Width = (FLOAT)m_shadowResolution;
	m_viewport.Height = (FLOAT)m_shadowResolution;
	m_viewport.MinDepth = 0;
	m_viewport.MaxDepth = 1;

	m_dlConstBuff.create(D3D11_USAGE_DYNAMIC,MAX_DIRECTIONAL_LIGHTS);
	m_slConstBuff.create(D3D11_USAGE_DYNAMIC, MAX_POINT_LIGHTS);
}

void Engine::ShadowSystem::createPointLightShadowMaps(size_t amount)
{
	m_plDSVS.clear();
	m_plDSVS.resize(amount);

	auto device = D3D::GetInstance()->GetDevice();

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = m_shadowResolution;
	textureDesc.Height = m_shadowResolution;
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
		hr = device->CreateDepthStencilView(texture.Get(), &depthStencilViewDesc, &m_plDSVS[i]);
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

	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, &m_srvPointLigts);
	assert(SUCCEEDED(hr));
}

void Engine::ShadowSystem::Create2DShadowMaps(size_t amount, std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>>& dsvs, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv)
{
	if (amount == 0)
		return;

	dsvs.resize(amount);

	auto device = D3D::GetInstance()->GetDevice();

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = m_shadowResolution;
	textureDesc.Height = m_shadowResolution;
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
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2DArray.ArraySize = 1u;
	depthStencilViewDesc.Texture2DArray.MipSlice = (UINT)0;

	for (size_t i = 0; i < amount; i++)
	{
		depthStencilViewDesc.Texture2DArray.FirstArraySlice = (UINT)i;
		hr = device->CreateDepthStencilView(texture.Get(), &depthStencilViewDesc, dsvs[i].GetAddressOf());
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

	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, &srv);
	assert(SUCCEEDED(hr));
}
