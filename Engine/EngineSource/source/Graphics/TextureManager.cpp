#include "Graphics\TextureManager.h"
#include "Graphics\DDSTextureLoader11.h"

Engine::Texture::Texture(const wchar_t* name)
{
	HRESULT hr = DirectX::CreateDDSTextureFromFile(Engine::D3D::GetInstance()->GetDevice(), name, &m_texture, &m_srv);
	assert(SUCCEEDED(hr));
}

void Engine::Texture::CreateTexture(const wchar_t* name)
{
	HRESULT hr = DirectX::CreateDDSTextureFromFile(Engine::D3D::GetInstance()->GetDevice(), name, &m_texture, &m_srv);
	assert(SUCCEEDED(hr));
}

void Engine::Texture::BindTexture(UINT slot, UINT ShaderType) const
{
	if (ShaderType & shaderTypes::VS)
	{
		Engine::D3D::GetInstance()->GetContext()->VSSetShaderResources(slot, 1, m_srv.GetAddressOf());
	}
	if (ShaderType & shaderTypes::HS)
	{
		Engine::D3D::GetInstance()->GetContext()->HSSetShaderResources(slot, 1, m_srv.GetAddressOf());
	}
	if (ShaderType & shaderTypes::DS)
	{
		Engine::D3D::GetInstance()->GetContext()->DSSetShaderResources(slot, 1, m_srv.GetAddressOf());
	}
	if (ShaderType & shaderTypes::GS)
	{
		Engine::D3D::GetInstance()->GetContext()->GSSetShaderResources(slot, 1, m_srv.GetAddressOf());
	}
	if (ShaderType & shaderTypes::PS)
	{
		Engine::D3D::GetInstance()->GetContext()->PSSetShaderResources(slot, 1, m_srv.GetAddressOf());
	}
	if (ShaderType & shaderTypes::CS)
	{
		Engine::D3D::GetInstance()->GetContext()->CSSetShaderResources(slot, 1, m_srv.GetAddressOf());
	}
}

UINT Engine::Texture::getTextureWidth()
{
	ID3D11Texture2D* texture;
	HRESULT hr = m_texture->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture);
	assert(SUCCEEDED(hr));

	D3D11_TEXTURE2D_DESC textDesc;
	texture->GetDesc(&textDesc);

	texture->Release();
	return textDesc.Width;
}

UINT Engine::Texture::getTextureHeight()
{
	ID3D11Texture2D* texture;
	HRESULT hr = m_texture->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture);
	assert(SUCCEEDED(hr));

	D3D11_TEXTURE2D_DESC textDesc;
	texture->GetDesc(&textDesc);

	texture->Release();
	return textDesc.Height;
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
	//if (m_textures.find(name) != m_textures.end())
	//	throw std::runtime_error("Texture with name '" + std::string(name) + "' already exists");

	if (m_textures.find(name) != m_textures.end())
		return m_textures[name];

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
	context->CSSetSamplers(0, 3, samplers);
	context->PSSetSamplers(4, 1, m_pointSamplareState.GetAddressOf());
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

void Engine::TextureManager::BindComparisonSampler(UINT slot)
{
	D3D::GetInstance()->GetContext()->PSSetSamplers(slot, 1, m_compsampler.GetAddressOf());
}

Engine::TextureManager::TextureManager()
{
	auto d3d = D3D::GetInstance();

	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
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

	d3d->GetContext()->PSSetSamplers(3u, 1, m_pointSamplareState.GetAddressOf());

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

	hr = D3D::GetInstance()->GetDevice()->CreateSamplerState(&sampDesc, &m_compsampler);
	assert(SUCCEEDED(hr));
}

