#include "Graphics/Renderer.h"
#include "Graphics/Model.h"
#include "Graphics/MeshSystem.h"
#include "Graphics/PostProcess.h"
#include "Graphics/LightSystem.h"
#include "Render/Camera.h"

std::mutex Engine::Renderer::mutex_;
Engine::Renderer* Engine::Renderer::pInstance = nullptr;

Engine::Renderer* Engine::Renderer::Init()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!pInstance)
    {
        pInstance = new Renderer();
    }

    return pInstance;
}

void Engine::Renderer::Deinit()
{
    delete pInstance;
    pInstance = nullptr;
}


void Engine::Renderer::InitDepthWithRTV(ID3D11Resource* RenderBuffer, UINT wWidth, UINT wHeight)
{
	if (auto device = D3D::GetInstance()->GetDevice())
	{
		HRESULT hr = device->CreateRenderTargetView(RenderBuffer, nullptr, &pRenderTarget);
		assert(SUCCEEDED(hr));

		Microsoft::WRL::ComPtr<ID3D11Texture2D> HDRtexture;

		D3D11_TEXTURE2D_DESC hdrDesc;
		ZeroMemory(&hdrDesc, sizeof(D3D11_TEXTURE2D_DESC));
		hdrDesc.Height = wHeight;
		hdrDesc.Width = wWidth;
		hdrDesc.MipLevels = 1u;
		hdrDesc.ArraySize = 1u;
		hdrDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		hdrDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		hdrDesc.SampleDesc.Count = 1u;
		hdrDesc.Usage = D3D11_USAGE_DEFAULT;

		hr = device->CreateTexture2D(&hdrDesc, nullptr, &HDRtexture);
		assert(SUCCEEDED(hr));

		hr = device->CreateRenderTargetView(HDRtexture.Get(), nullptr, &pHDRRenderTarget);
		assert(SUCCEEDED(hr));

		hr = device->CreateShaderResourceView(HDRtexture.Get(), nullptr, &pHDRtextureResource);
		assert(SUCCEEDED(hr));


		InitDepth(wWidth, wHeight);


		perViewBuffer.bind(0u, shaderTypes::VS | shaderTypes::PS | shaderTypes::DS | shaderTypes::GS);
		perFrameBuffer.bind(1u, shaderTypes::VS | shaderTypes::PS | shaderTypes::GS);
	}
}

void Engine::Renderer::InitDepth(UINT wWidth, UINT wHeight)
{
	D3D* d3d = D3D::GetInstance();
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;

	HRESULT hr = d3d->GetDevice()->CreateDepthStencilState(&dsDesc, &pDSState);
	assert(SUCCEEDED(hr));

	d3d->GetContext()->OMSetDepthStencilState(pDSState.Get(), 1u);

	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = (UINT)wWidth;
	descDepth.Height = (UINT)wHeight;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = d3d->GetDevice()->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);
	assert(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC descDVS;
	ZeroMemory(&descDVS, sizeof(descDVS));
	descDVS.Format = DXGI_FORMAT_D32_FLOAT;
	descDVS.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDVS.Texture2D.MipSlice = 0u;

	hr = d3d->GetDevice()->CreateDepthStencilView(pDepthStencil.Get(), &descDVS, &pViewDepth);
	assert(SUCCEEDED(hr));


}

void Engine::Renderer::updatePerFrameCB(float deltaTime, float wWidth, float wHeight)
{
	perFrameData.g_time += deltaTime;

	if (perFrameData.g_resolution[0] != wWidth || perFrameData.g_resolution[1] != wHeight)
	{
		perFrameData.g_resolution[0] = wWidth;
		perFrameData.g_resolution[1] = wHeight;
		perFrameData.g_resolution[2] = perFrameData.g_resolution[3] = 1 / (wWidth * wHeight);
	}

	perFrameBuffer.updateBuffer(&perFrameData);
}

void Engine::Renderer::Render(Camera* camera)
{
	Engine::D3D* d3d = Engine::D3D::GetInstance();


	d3d->GetContext()->OMSetRenderTargets(1u, pHDRRenderTarget.GetAddressOf(), pViewDepth.Get());
	d3d->GetContext()->OMSetDepthStencilState(pDSState.Get(), 1u);

	static const float color[] = { 0.5f, 0.5f,0.5f,1.0f };
	
	d3d->GetContext()->ClearRenderTargetView(pHDRRenderTarget.Get(), color);
	d3d->GetContext()->ClearDepthStencilView(pViewDepth.Get(), D3D11_CLEAR_DEPTH, 0.0f, 0u);

	vec3 temp = cross(camera->getForward(), camera->getRight());

	PerViewCB perView = PerViewCB{ camera->getViewMatrix() * camera->getProjectionMatrix(), vec4(camera->getCameraFrustrum(Camera::LeftDown),.0f),
		vec4(camera->getCameraFrustrum(Camera::LeftUp) - camera->getCameraFrustrum(Camera::LeftDown),.0f),
		vec4(camera->getCameraFrustrum(Camera::RightDown) - camera->getCameraFrustrum(Camera::LeftDown),.0f),
		camera->getPosition()};

	perViewBuffer.updateBuffer(&perView);
	LightSystem::Init()->BindLigtsBuffer(3u, shaderTypes::PS);

	if (isIBLLighOn)
	{
		diffuseIBL->BindTexture(6u);
		specularIBL->BindTexture(7u);
		reflectanceIBL->BindTexture(8u);
	}
	Engine::LightSystem::Init()->UpdateLightsBuffer();
	Engine::LightSystem::Init()->BindLightTextures();

	MeshSystem::Init()->render();
}

void Engine::Renderer::PostProcess()
{
	
	PostProcess::Init()->Resolve(pHDRtextureResource.Get(), pRenderTarget.Get());
}

void Engine::Renderer::setIBLLight(std::shared_ptr<Texture> diffuse, std::shared_ptr<Texture> specular, std::shared_ptr<Texture> reflectance)
{
	diffuseIBL = diffuse;
	specularIBL = specular;
	reflectanceIBL = reflectance;
	isIBLLighOn = true;
}

void Engine::Renderer::setIBLLghtState(bool state)
{
	isIBLLighOn = state;
}

Engine::Renderer::Renderer() :
	perFrameData()
{
    perFrameBuffer.create();
    perViewBuffer.create();
}
