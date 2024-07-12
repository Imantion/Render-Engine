#include "ReflectionCapture.h"
#include "Math/vec.h"
#include "DirectXTex.h"
#include "Buffers.h"
#include "ShaderManager.h"

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

void ReflectionCapture::GenerateCubeMap(ID3D11RenderTargetView* rtv, ID3D11Texture2D* tex, UINT width, UINT height)
{
	D3D11_TEXTURE2D_DESC textDesc = {};
	textDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textDesc.Width = width;
	textDesc.Height = height;
	textDesc.MipLevels = 1;
	textDesc.ArraySize = 6;
	textDesc.SampleDesc.Count = 1;
	textDesc.SampleDesc.Quality = 0;
	textDesc.Usage = D3D11_USAGE_DEFAULT;
	textDesc.CPUAccessFlags = 0;
	textDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hr = Engine::D3D::GetInstance()->GetDevice()->CreateTexture2D(&textDesc, nullptr, &tex);
	assert(SUCCEEDED(hr));

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 6;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.FirstArraySlice = 0;

	hr = Engine::D3D::GetInstance()->GetDevice()->CreateRenderTargetView(tex, &rtvDesc, &rtv);
	assert(SUCCEEDED(hr));
	

	struct cl
	{
		Engine::vec4 normal[6];
		Engine::vec4 color[6];
	};

	cl data = { { Engine::vec4(1,0,0,1), Engine::vec4(-1,0,0,1), Engine::vec4(0,1,0,1),Engine::vec4(0,-1,0,1),Engine::vec4(0,0,1,1),Engine::vec4(0,0,-1,1) },
	{ Engine::vec4(1,0,0,1), Engine::vec4(0,1,0,1), Engine::vec4(1,1,0,1),Engine::vec4(0,0,1,1),Engine::vec4(1,0,1,1),Engine::vec4(0,1,1,1) } };

	Engine::ConstBuffer<cl> cb;

	auto shader = Engine::ShaderManager::CompileAndCreateShader("TEST", L"Shaders\\IBLcreation\\CubeMapVS.hlsl", L"Shaders\\IBLcreation\\CubeMapPS.hlsl",
		nullptr, nullptr, L"Shaders\\IBLcreation\\CubeMapGS.hlsl", nullptr, nullptr);
	cb.create();

	auto deviceContext = Engine::D3D::GetInstance()->GetContext();

	D3D11_VIEWPORT viewPort = {};
	viewPort.Width = width;
	viewPort.Height = height;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;

	shader->BindShader();
	
	float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	deviceContext->ClearRenderTargetView(rtv, clearColor);
	deviceContext->OMSetRenderTargets(1, &rtv, nullptr);
	deviceContext->RSSetViewports(1, &viewPort);

	cb.updateBuffer(&data);

	cb.bind(0u, Engine::shaderTypes::GS);

	deviceContext->Draw(3u, 0u);
	

	saveCapture(L"TEST.dds", Engine::D3D::GetInstance()->GetDevice(), deviceContext, tex, false, FileFormat::BC6_UNSIGNED);
}
