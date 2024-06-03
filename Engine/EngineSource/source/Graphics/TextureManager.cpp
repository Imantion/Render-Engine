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


std::unordered_map<std::string, std::shared_ptr<Engine::Texture>> Engine::TextureManager::m_textures;
std::mutex Engine::TextureManager::m_mutex;
Engine::TextureManager* Engine::TextureManager::m_Instance;

Engine::TextureManager* Engine::TextureManager::Init()
{
	std::lock_guard<std::mutex> m(m_mutex);

	if (!m_Instance)
	{
		m_Instance = new TextureManager();
	}

	return m_Instance;
}

void Engine::TextureManager::Deinit()
{
	delete m_Instance;
	m_textures.clear();
}

Engine::Texture* Engine::TextureManager::AddTexture(const char* name, const wchar_t* path)
{
	if (m_textures.find(name) != m_textures.end())
		throw std::runtime_error("Texture with name '" + std::string(name) + "' already exists");

	auto texture = std::make_shared<Texture>(path);
	m_textures.emplace(name, texture);

	return texture.get();
}

Engine::Texture* Engine::TextureManager::GetTexture(const char* name)
{
	auto iter = m_textures.find(name);

	if(iter == m_textures.end())
		return nullptr;

	return (*iter).second.get();
}

