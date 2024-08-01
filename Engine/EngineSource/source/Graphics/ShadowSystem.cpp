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
	createSpotLightShadowMaps(m_slDSVS.size());
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

void Engine::ShadowSystem::BindShadowTextures(UINT pointLightTextureSlot , UINT spotLightTextureSlot)
{
	auto context = D3D::GetInstance()->GetContext();
	
	context->PSSetShaderResources(pointLightTextureSlot, 1u, m_srvPointLigts.GetAddressOf());
	context->PSSetShaderResources(spotLightTextureSlot, 1u, m_srvSpotLigts.GetAddressOf());
}

Engine::ShadowSystem::ShadowSystem()
{
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_viewport.Width = (FLOAT)m_shadowResolution;
	m_viewport.Height = (FLOAT)m_shadowResolution;
	m_viewport.MinDepth = 0;
	m_viewport.MaxDepth = 1;
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

void Engine::ShadowSystem::createSpotLightShadowMaps(size_t amount)
{
	m_slDSVS.resize(amount);
	m_slViewProjections.resize(amount);

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
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	depthStencilViewDesc.Texture2DArray.ArraySize = 1u;
	depthStencilViewDesc.Texture2DArray.MipSlice = (UINT)0;

	for (size_t i = 0; i < amount; i++)
	{
		depthStencilViewDesc.Texture2DArray.FirstArraySlice = (UINT)i;
		hr = device->CreateDepthStencilView(texture.Get(), &depthStencilViewDesc, m_slDSVS[i].GetAddressOf());
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

	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, &m_srvSpotLigts);
	assert(SUCCEEDED(hr));
}
