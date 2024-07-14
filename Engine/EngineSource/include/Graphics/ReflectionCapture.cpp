#include "ReflectionCapture.h"
#include "Math/vec.h"
#include "Math/matrix.h"
#include "Math/math.h"
#include "DirectXTex.h"
#include "Buffers.h"
#include "ShaderManager.h"
#include "TextureManager.h"

#define BREAK __debugbreak();

#define ALWAYS_ASSERT(expression, ...) \
	if (!(expression)) \
	{ \
		BREAK; \
		std::abort(); \
	} else {}
// else block is needed so external else doesn't relate to the if block in the macro in this case:
// if (condition) ALWAYS_ASSERT(expression);
// else { ... }

#ifdef NDEBUG
#define DEV_ASSERT(...)
#else
#define DEV_ASSERT(expression, ...) ALWAYS_ASSERT(expression, __VA_ARGS__);
#endif

using namespace Engine;

void ReflectionCapture::saveCapture(const wchar_t* filename, ID3D11Device* s_device, ID3D11DeviceContext* s_devcon, ID3D11Texture2D* tex, bool generateMips, FileFormat format)
{
	DirectX::ScratchImage scratchImage;
	DirectX::CaptureTexture(s_device, s_devcon, tex, scratchImage);

	const DirectX::ScratchImage* imagePtr = &scratchImage;

	DirectX::ScratchImage mipchain;
	if (generateMips)
	{
		DirectX::GenerateMipMaps(*scratchImage.GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, 0, mipchain);
		imagePtr = &mipchain;
	}

	DirectX::ScratchImage compressed;
	if (DirectX::IsCompressed(DXGI_FORMAT(format)))
	{
		HRESULT result;
		if (FileFormat::BC6_UNSIGNED <= format && format <= FileFormat::BC7_SRGB)
			result = DirectX::Compress(s_device, imagePtr->GetImages(), imagePtr->GetImageCount(), imagePtr->GetMetadata(),
				DXGI_FORMAT(format), DirectX::TEX_COMPRESS_PARALLEL, 1.f, compressed);
		else
			result = DirectX::Compress(imagePtr->GetImages(), imagePtr->GetImageCount(), imagePtr->GetMetadata(),
				DXGI_FORMAT(format), DirectX::TEX_COMPRESS_PARALLEL, 1.f, compressed);

		DEV_ASSERT(result >= 0);
		imagePtr = &compressed;
	}

	DirectX::SaveToDDSFile(imagePtr->GetImages(), imagePtr->GetImageCount(), imagePtr->GetMetadata(), DirectX::DDS_FLAGS(0), filename);
}



void ReflectionCapture::GenerateCubeMap(const wchar_t* psShaderPath, ID3D11Texture2D** tex, UINT resolution, UINT numbersOfSample, UINT maxMipLevel)
{
	if (maxMipLevel != 0 && (resolution >> maxMipLevel) < 1)
		throw "Max mip level is not compatible with given resolution";

	UINT mipLevels = maxMipLevel + 1;
	float roughnessStep = maxMipLevel == 0 ? 0.0f : 1.0f / (float)maxMipLevel;

	D3D11_TEXTURE2D_DESC textDesc = {};
	textDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textDesc.Width = resolution;
	textDesc.Height = resolution;
	textDesc.MipLevels = mipLevels;
	textDesc.ArraySize = 6;
	textDesc.SampleDesc.Count = 1;
	textDesc.SampleDesc.Quality = 0;
	textDesc.Usage = D3D11_USAGE_DEFAULT;
	textDesc.CPUAccessFlags = 0;
	textDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hr = Engine::D3D::GetInstance()->GetDevice()->CreateTexture2D(&textDesc, nullptr, tex);
	assert(SUCCEEDED(hr));

	std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> rtvs(mipLevels);
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 6;
	rtvDesc.Texture2DArray.FirstArraySlice = 0;
	for (size_t i = 0; i < mipLevels; i++)
	{
		rtvDesc.Texture2DArray.MipSlice = (UINT)i;
		hr = Engine::D3D::GetInstance()->GetDevice()->CreateRenderTargetView(*tex, &rtvDesc, &rtvs[i]);
		assert(SUCCEEDED(hr));
	}



	struct cubeFrustrum
	{
		Engine::vec4 frustrums[18];
	} gsCBdata;

	Engine::ConstBuffer<cubeFrustrum> gsCB;
	gsCB.create();

	mat4 projection = projectionMatrix(M_PI / 2.0f, 0.01f, 100.f, resolution, resolution);
	mat4 inverseView[6];
	Engine::vec3 pos(0.0f);
	inverseView[0] = mat4::Inverse(viewMatrix(pos, vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
	inverseView[1] = mat4::Inverse(viewMatrix(pos, vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
	inverseView[2] = mat4::Inverse(viewMatrix(pos, vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)));
	inverseView[3] = mat4::Inverse(viewMatrix(pos, vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
	inverseView[4] = mat4::Inverse(viewMatrix(pos, vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f)));
	inverseView[5] = mat4::Inverse(viewMatrix(pos, vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 1.0f)));
	vec2 invProj = vec2(1.0f / projection[0][0], 1.0f / projection[1][1]);
	for (size_t i = 0; i < 6; i++)
	{
		gsCBdata.frustrums[3 * i + 0] = vec4(-1.0f * invProj.x, -1.0f * invProj.y, 1.0f, 0.0f) * inverseView[i];
		gsCBdata.frustrums[3 * i + 1] = vec4(-1.0f * invProj.x, 3.0f * invProj.y, 1.0f, 0.0f) * inverseView[i];
		gsCBdata.frustrums[3 * i + 2] = vec4(3.0f * invProj.x, -1.0f * invProj.y, 1.0f, 0.0f) * inverseView[i];
	}
	gsCB.updateBuffer(&gsCBdata);
	gsCB.bind(0u, Engine::shaderTypes::GS);


	struct textureProcessInfo
	{
		UINT resolution;
		UINT numbersOfSample;
		float roughness;
		float padding[1];
	} psCBdata;
	psCBdata = { resolution, numbersOfSample };
	Engine::ConstBuffer<textureProcessInfo> psCB;
	psCB.create();


	auto shader = Engine::ShaderManager::CompileAndCreateShader("CubeMapIBL", L"Shaders\\ReflectionCapture\\CubeMapVS.hlsl", psShaderPath,
		nullptr, nullptr, L"Shaders\\ReflectionCapture\\CubeMapGS.hlsl", nullptr, nullptr);
	shader->BindShader();

	auto deviceContext = Engine::D3D::GetInstance()->GetContext();
	TextureManager::Init()->BindSamplers();

	for (size_t i = 0; i < mipLevels; i++)
	{
		D3D11_VIEWPORT viewPort = {};
		viewPort.Width = (float)(resolution >> i);
		viewPort.Height = (float)(resolution >> i);
		viewPort.MinDepth = 0;
		viewPort.MaxDepth = 1;

		psCBdata.roughness = roughnessStep * (float)i;
		psCB.updateBuffer(&psCBdata);
		psCB.bind(2u, Engine::shaderTypes::PS);

		float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
		deviceContext->ClearRenderTargetView(rtvs[i].Get(), clearColor);
		deviceContext->OMSetRenderTargets(1, rtvs[i].GetAddressOf(), nullptr);
		deviceContext->RSSetViewports(1, &viewPort);
		deviceContext->Draw(3u, 0u);

	}
}

void ReflectionCapture::IBLdiffuse(const wchar_t* generatedTextureName, std::shared_ptr<Texture> source, UINT numberOfSamples)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
	source->BindTexture(0u);
	GenerateCubeMap(L"Shaders\\ReflectionCapture\\IBLdiffusePS.hlsl", &tex, source->getTextureWidth(), numberOfSamples, 0u);
	saveCapture(generatedTextureName, Engine::D3D::GetInstance()->GetDevice(), Engine::D3D::GetInstance()->GetContext(), tex.Get(), false, FileFormat::BC6_UNSIGNED);
}

void Engine::ReflectionCapture::IBLspecularIrradiance(const wchar_t* generatedTextureName, std::shared_ptr<Texture> source, UINT numberOfSamples, UINT maxMipLevel)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
	source->BindTexture(0u);
	GenerateCubeMap(L"Shaders\\ReflectionCapture\\IBLspecularIrradiancePS.hlsl", &tex, source->getTextureWidth(), numberOfSamples, maxMipLevel);
	saveCapture(generatedTextureName, Engine::D3D::GetInstance()->GetDevice(), Engine::D3D::GetInstance()->GetContext(), tex.Get(), false, FileFormat::BC6_UNSIGNED);
}

void Engine::ReflectionCapture::IBLreflectance(const wchar_t* generatedTextureName, std::shared_ptr<Texture> source,UINT resolution, UINT numberOfSamples)
{
	auto device = D3D::Init()->GetDevice();
	auto context = D3D::Init()->GetContext();

	source->BindTexture(0u);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
	D3D11_TEXTURE2D_DESC textDesc = {};
	textDesc.ArraySize = 1;
	textDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	textDesc.Width = resolution;
	textDesc.Height = resolution;
	textDesc.MipLevels = 1;
	textDesc.SampleDesc.Count = 1;
	textDesc.SampleDesc.Quality = 0;
	textDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	HRESULT hr = device->CreateTexture2D(&textDesc, nullptr, &tex);
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(tex.Get(), &rtvDesc, &rtv);

	auto shader = Engine::ShaderManager::CompileAndCreateShader("IBlreflectance", L"Shaders\\ReflectionCapture\\IBLReflectanceVS.hlsl", L"Shaders\\ReflectionCapture\\IBLReflectancePS.hlsl", nullptr, nullptr);
	shader->BindShader();

	struct textureProcessInfo
	{
		UINT resolution;
		UINT numbersOfSample;
		float roughness;
		float padding[1];
	} psCBdata;
	psCBdata = { resolution, numberOfSamples };
	Engine::ConstBuffer<textureProcessInfo> psCB;
	psCB.create();
	psCB.updateBuffer(&psCBdata);
	psCB.bind(2u, shaderTypes::PS);

	D3D11_VIEWPORT viewport = {};
	viewport.Width = resolution;
	viewport.Height = resolution;
	viewport.MaxDepth = 1.0f;

	float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	context->ClearRenderTargetView(rtv.Get(), clearColor);
	context->OMSetRenderTargets(1, rtv.GetAddressOf(), nullptr);
	context->RSSetViewports(1, &viewport);
	context->Draw(3u, 0u);

	saveCapture(generatedTextureName, device, context, tex.Get(), false, FileFormat::BC5_UNSIGNED);
}
