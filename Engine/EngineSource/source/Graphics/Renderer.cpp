#include "Graphics/Renderer.h"
#include "Graphics/Model.h"
#include "Graphics/MeshSystem.h"
#include "Graphics/PostProcess.h"
#include "Graphics/LightSystem.h"
#include "Graphics/ShadowSystem.h"
#include "Graphics/ParticleSystem.h"
#include "Graphics/SkyBox.h"
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
		hdrDesc.SampleDesc.Count = samplesAmount;
		hdrDesc.SampleDesc.Quality = 0u;
		hdrDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
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
	descDepth.SampleDesc.Count = samplesAmount;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = d3d->GetDevice()->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);
	assert(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC descDVS;
	ZeroMemory(&descDVS, sizeof(descDVS));
	descDVS.Format = DXGI_FORMAT_D32_FLOAT;
	descDVS.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	descDVS.Texture2D.MipSlice = 0u;

	hr = d3d->GetDevice()->CreateDepthStencilView(pDepthStencil.Get(), &descDVS, &pViewDepth);
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = (UINT)wWidth;
	textureDesc.Height = (UINT)wHeight;
	textureDesc.MipLevels = 1u;
	textureDesc.ArraySize = 1u;
	textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;

	hr = d3d->GetDevice()->CreateTexture2D(&textureDesc, nullptr, &texture);
	assert(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2DArray.ArraySize = 1u;
	depthStencilViewDesc.Texture2DArray.MipSlice = (UINT)0;

	hr = d3d->GetDevice()->CreateDepthStencilView(texture.Get(), &depthStencilViewDesc, &pNoMSDepthStencil);
	assert(SUCCEEDED(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));

	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2DArray.ArraySize = 1;
	srvDesc.Texture2DArray.FirstArraySlice = 0u;
	srvDesc.Texture2DArray.MipLevels = 1u;
	srvDesc.Texture2DArray.MostDetailedMip = 0u;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

	hr = d3d->GetDevice()->CreateShaderResourceView(texture.Get(), &srvDesc, &pDepthSRV);
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

	perFrameData.shadowResolution = (float)ShadowSystem::Init()->GetShadowTextureResolution();
	perFrameData.pointLightFarPlan = ShadowSystem::Init()->GetProjectionFarPlane();
	perFrameData.samplesAmount = samplesAmount;

	perFrameBuffer.updateBuffer(&perFrameData);
}

void Engine::Renderer::Render(Camera* camera)
{
	auto context = Engine::D3D::GetInstance()->GetContext();

	context->OMSetDepthStencilState(pDSState.Get(), 1u);
	context->OMSetBlendState(pBlendState.Get(), nullptr, 0xFFFFFFFF);

	Shadows(camera);
	
	context->OMSetRenderTargets(1u, pHDRRenderTarget.GetAddressOf(), pViewDepth.Get());
	

	static const float color[] = { 0.5f, 0.5f,0.5f,1.0f };
	
	context->ClearRenderTargetView(pHDRRenderTarget.Get(), color);
	context->ClearDepthStencilView(pViewDepth.Get(), D3D11_CLEAR_DEPTH, 0.0f, 0u);


	PerViewCB perView = PerViewCB{ camera->getViewMatrix() * camera->getProjectionMatrix(), vec4(camera->getCameraFrustrum(Camera::LeftDown),.0f),
		vec4(camera->getCameraFrustrum(Camera::LeftUp) - camera->getCameraFrustrum(Camera::LeftDown),.0f),
		vec4(camera->getCameraFrustrum(Camera::RightDown) - camera->getCameraFrustrum(Camera::LeftDown),.0f),
		camera->getPosition()};

	
	perViewBuffer.updateBuffer(&perView);

	LightSystem::Init()->BindLigtsBuffer(3u, shaderTypes::PS);

	if (perFrameData.IBL)
	{
		diffuseIBL->BindTexture(6u);
		specularIBL->BindTexture(7u);
		reflectanceIBL->BindTexture(8u);
	}

	if (perFrameData.LTC)
	{
		LTCmat->BindTexture(9u);
		LTCamp->BindTexture(10u);
	}

	LightSystem::Init()->UpdateLightsBuffer();
	LightSystem::Init()->BindLightTextures();
	TextureManager::Init()->BindComparisonSampler(5u);

	MeshSystem::Init()->render();
	pSkyBox->BindSkyBox(2u);
	pSkyBox->Draw();

	context->OMSetDepthStencilState(pDSState.Get(), 1u);
	MeshSystem::Init()->renderTranslucent();

	ParticleSystem::Init()->Update(0.016667f);
	ParticleSystem::Init()->UpdateBuffers(camera->getPosition());
	ParticleSystem::Init()->Render();

	ID3D11ShaderResourceView* const pSRV[3] = { NULL, NULL, NULL };
	context->PSSetShaderResources(11, 3u, pSRV);
}

void Engine::Renderer::PostProcess()
{
	D3D::GetInstance()->GetContext()->RSSetState(nullptr);
	PostProcess::Init()->Resolve(pHDRtextureResource.Get(), pRenderTarget.Get());
}

void Engine::Renderer::setSkyBox(std::shared_ptr<SkyBox> skybox)
{
	pSkyBox = skybox;
}

void Engine::Renderer::setIBLLight(std::shared_ptr<Texture> diffuse, std::shared_ptr<Texture> specular, std::shared_ptr<Texture> reflectance)
{
	diffuseIBL = diffuse;
	specularIBL = specular;
	reflectanceIBL = reflectance;
	perFrameData.IBL = true;
}

void Engine::Renderer::setLTCLight(std::shared_ptr<Texture> invMatrix, std::shared_ptr<Texture> amplitude)
{
	LTCmat = invMatrix;
	LTCamp = amplitude;
	perFrameData.LTC = true;
}

void Engine::Renderer::setIBLLghtState(bool state)
{
	perFrameData.IBL = state;
}

void Engine::Renderer::setDiffuseState(bool state)
{
	perFrameData.diffuse = state;
}

void Engine::Renderer::setSpecularState(bool state)
{
	perFrameData.specular = state;
}

void Engine::Renderer::setLTCState(bool state)
{
	perFrameData.LTC = state;
}

Engine::Renderer::Renderer() :
	perFrameData()
{
    perFrameBuffer.create();
    perViewBuffer.create();

	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.DepthBias = -64;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = -1.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.AntialiasedLineEnable = false;

	HRESULT hr = D3D::GetInstance()->GetDevice()->CreateRasterizerState(&rasterDesc, &pRasterizerState);
	assert(SUCCEEDED(hr));

	rasterDesc.DepthBias = 0;
	rasterDesc.SlopeScaledDepthBias = 0;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.MultisampleEnable = true;

	hr = D3D::GetInstance()->GetDevice()->CreateRasterizerState(&rasterDesc, &MSAARasterizerState);

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend =
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend =
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp =
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = D3D::GetInstance()->GetDevice()->CreateBlendState(&blendDesc, &pBlendState);

}

void Engine::Renderer::Shadows(const Camera* camera)
{
	auto context = D3D::GetInstance()->GetContext();

	context->RSSetState(pRasterizerState.Get());
	std::vector<Engine::vec3> positions;
	Engine::LightSystem::Init()->GetPointLightsPositions(positions);
	Engine::MeshSystem::Init()->renderDepthCubemaps(positions);

	std::vector<SpotLight> sl;
	sl.push_back(Engine::LightSystem::Init()->getFlashLight());
	Engine::MeshSystem::Init()->renderDepth2D(sl);

	Engine::MeshSystem::Init()->renderDepth2DDirectional(LightSystem::Init()->GetDirectionalLights(), camera);

	context->RSSetState(MSAARasterizerState.Get());

	ShadowSystem::Init()->BindShadowTextures(11u, 12u, 13u);
	ShadowSystem::Init()->BindShadowBuffers(5u, 6u);
}
