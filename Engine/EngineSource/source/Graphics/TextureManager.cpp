#include "Graphics\TextureManager.h"
#include "Graphics\DDSTextureLoader11.h"

Engine::Texture::Texture(const wchar_t* name)
{
	HRESULT hr = DirectX::CreateDDSTextureFromFile(Engine::D3D::GetInstance()->GetDevice(), name, nullptr, &m_srv);
	assert(SUCCEEDED(hr));
}

void Engine::Texture::CreateTexture(const wchar_t* name)
{
	HRESULT hr = DirectX::CreateDDSTextureFromFile(Engine::D3D::GetInstance()->GetDevice(), name, nullptr, &m_srv);
	assert(SUCCEEDED(hr));
}

void Engine::Texture::BindTexture(UINT slot) const
{
	D3D::GetInstance()->GetContext()->PSSetShaderResources(slot, 1u, m_srv.GetAddressOf());
}

/// ////////////////////////////////////////////////////// Texture Manager

std::mutex Engine::TextureManager::m_mutex;
Engine::TextureManager* Engine::TextureManager::m_instance;

Engine::TextureManager* Engine::TextureManager::Init()
{
	std::lock_guard<std::mutex> m(m_mutex);

	if (!m_instance)
	{
		m_instance = new TextureManager();
	}

	return m_instance;
}

void Engine::TextureManager::Deinit()
{
	delete m_instance;
	m_instance = nullptr;
}

std::shared_ptr<Engine::Texture> Engine::TextureManager::LoadFromFile(const char* name, const wchar_t* path)
{
	if (m_textures.find(name) != m_textures.end())
		throw std::runtime_error("Texture with name '" + std::string(name) + "' already exists");

	auto texture = std::make_shared<Texture>(path);
	m_textures.emplace(name, texture);

	return texture;
}

std::shared_ptr<Engine::Texture> Engine::TextureManager::GetTexture(const char* name)
{
	auto iter = m_textures.find(name);

	if(iter == m_textures.end())
		return nullptr;

	return (*iter).second;
}

void Engine::TextureManager::BindSamplers()
{
	auto context = D3D::GetInstance()->GetContext();

	static ID3D11SamplerState* samplers[3] = { m_pointSamplareState.Get(), m_linearSamplareState.Get(), m_anisotropicSamplareState.Get() };

	context->PSSetSamplers(0, 3, samplers);
}

void Engine::TextureManager::BindSampleByFilter(D3D11_FILTER filter, UINT slot)
{
	auto context = D3D::GetInstance()->GetContext();
	switch (filter)
	{
	case D3D11_FILTER_MIN_MAG_MIP_POINT:
		context->PSSetSamplers(slot, 1, m_pointSamplareState.GetAddressOf());
		break;
	case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
		context->PSSetSamplers(slot, 1, m_linearSamplareState.GetAddressOf());
		break;
	case D3D11_FILTER_ANISOTROPIC:
		context->PSSetSamplers(slot, 1, m_anisotropicSamplareState.GetAddressOf());
		break;
	default:
		break;
	}
}

Engine::TextureManager::TextureManager()
{
	auto d3d = D3D::GetInstance();

	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT hr =  d3d->GetDevice()->CreateSamplerState(&desc, &m_pointSamplareState);
	assert(SUCCEEDED(hr));

	desc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	hr = d3d->GetDevice()->CreateSamplerState(&desc, &m_linearSamplareState);
	assert(SUCCEEDED(hr));

	desc.Filter = D3D11_FILTER_ANISOTROPIC;
	desc.MaxAnisotropy = 8;
	hr = d3d->GetDevice()->CreateSamplerState(&desc, &m_anisotropicSamplareState);
	assert(SUCCEEDED(hr));
}

