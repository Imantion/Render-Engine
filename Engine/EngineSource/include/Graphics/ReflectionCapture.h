#pragma once
#include "Graphics/D3D.h"
#include <memory>

namespace Engine
{
	class Texture;

	class ReflectionCapture
	{
	public:
		enum class FileFormat
		{
			NONE,
			PNG,
			TGA,
			HDR,
			BC1_LINEAR = DXGI_FORMAT_BC1_UNORM,			// RGB, 1 bit Alpha
			BC1_SRGB = DXGI_FORMAT_BC1_UNORM_SRGB,		// RGB, 1-bit Alpha, SRGB
			BC3_LINEAR = DXGI_FORMAT_BC3_UNORM,			// RGBA
			BC3_SRGB = DXGI_FORMAT_BC3_UNORM_SRGB,		// RGBA, SRGB
			BC4_UNSIGNED = DXGI_FORMAT_BC4_UNORM,		// GRAY, unsigned
			BC4_SIGNED = DXGI_FORMAT_BC4_SNORM,			// GRAY, signed
			BC5_UNSIGNED = DXGI_FORMAT_BC5_UNORM,		// RG, unsigned
			BC5_SIGNED = DXGI_FORMAT_BC5_SNORM,			// RG, signed
			BC6_UNSIGNED = DXGI_FORMAT_BC6H_UF16,		// RGB HDR, unsigned
			BC6_SIGNED = DXGI_FORMAT_BC6H_SF16,			// RGB HDR, signed
			BC7_LINEAR = DXGI_FORMAT_BC7_UNORM,			// RGBA Advanced
			BC7_SRGB = DXGI_FORMAT_BC7_UNORM_SRGB,		// RGBA Advanced, SRGB
		};

	public:

		static void saveCapture(const wchar_t* filename, ID3D11Device* s_device, ID3D11DeviceContext* s_devcon, ID3D11Texture2D* tex, bool generateMips, FileFormat format);
		static void IBLdiffuse(const wchar_t* generatedTextureName, std::shared_ptr<Texture> source, UINT numberOfSamples);
		static void IBLspecularIrradiance(const wchar_t* generatedTextureName, std::shared_ptr<Texture> source, UINT numberOfSamples, UINT maxMipLevel);
		static void IBLreflectance(const wchar_t* generatedTextureName, std::shared_ptr<Texture> source,UINT resolution, UINT numberOfSamples);
		static void GenerateCubeMap(const wchar_t* psShaderPath, ID3D11Texture2D** tex, UINT resolution, UINT numbersOfSample, UINT maxMipLevel);
	private:

	};
}




