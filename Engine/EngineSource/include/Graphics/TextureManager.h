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
		TextureManager(const TextureManager& other) = delete;
		void operator=(const TextureManager& other) = delete;
		
		static TextureManager* Init();
		static void Deinit();

		std::shared_ptr<Texture> LoadFromFile(const char* name, const wchar_t* path);

		std::shared_ptr<Texture> GetTexture(const char* name);
		
		void BindSamplers();
		void BindSampleByFilter(D3D11_FILTER filter, UINT slot);

	protected:
		TextureManager();
		~TextureManager() = default;

	private:
		static TextureManager* m_instance;
		std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;

		static std::mutex m_mutex;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pointSamplareState;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_linearSamplareState;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_anisotropicSamplareState;

	};


}