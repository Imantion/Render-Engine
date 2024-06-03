#pragma once
#include "D3D.h"
#include <unordered_map>
#include <memory>

namespace Engine
{

	class Texture
	{
	public:
		Texture() = default;
		Texture(const wchar_t* name);

		void CreateTexture(const wchar_t* name);

		void BindTexture(UINT slot) const;
		
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
	};

	class TextureManager
	{
	public:

		static TextureManager* Init();
		static void Deinit();

		Texture* AddTexture(const char* name, const wchar_t* path);

		Texture* GetTexture(const char* name);

	protected:
		TextureManager() = default;

	private:
		static TextureManager* m_Instance;

		static std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
		static std::mutex m_mutex;

	private:
		TextureManager(TextureManager& other) = delete;
		void operator=(const TextureManager&) = delete;

	};


}