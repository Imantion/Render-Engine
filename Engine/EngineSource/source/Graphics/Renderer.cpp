#include "Graphics/Renderer.h"
#include "Graphics/Model.h"
#include "Graphics/MeshSystem.h"
#include "Graphics/PostProcess.h"
#include "Graphics/LightSystem.h"
#include "Graphics/ShadowSystem.h"
#include "Graphics/ParticleSystem.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/DecalSystem.h"
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
		InitGBuffer(wWidth, wHeight);

		perFrameData.texelWidth = 1.0f / (float)wWidth;
		perFrameData.texelHeight = 1.0f / (float)wHeight;

		perViewBuffer.bind(0u, shaderTypes::VS | shaderTypes::PS | shaderTypes::GS);
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
	dsDesc.DepthFunc = D3D11_COMPARISON_GREATER;

	// Stencil test parameters
	dsDesc.StencilEnable = TRUE;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;

	HRESULT hr = d3d->GetDevice()->CreateDepthStencilState(&dsDesc, &pDSState);
	assert(SUCCEEDED(hr));

	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = d3d->GetDevice()->CreateDepthStencilState(&dsDesc, &pDSReadOnlyState);
	assert(SUCCEEDED(hr));


	d3d->GetContext()->OMSetDepthStencilState(pDSState.Get(), 1u);


	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = FALSE;
	dsDesc.StencilEnable = TRUE;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;

	hr = d3d->GetDevice()->CreateDepthStencilState(&dsDesc, &pDSStencilOnlyState);
	assert(SUCCEEDED(hr));

	
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = (UINT)wWidth;
	descDepth.Height = (UINT)wHeight;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
	descDepth.SampleDesc.Count = samplesAmount;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	hr = d3d->GetDevice()->CreateTexture2D(&descDepth, nullptr, &pDepthStencilTexture);
	assert(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC descDVS;
	ZeroMemory(&descDVS, sizeof(descDVS));
	descDVS.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDVS.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDVS.Texture2D.MipSlice = 0u;

	hr = d3d->GetDevice()->CreateDepthStencilView(pDepthStencilTexture.Get(), &descDVS, &pViewDepth);
	assert(SUCCEEDED(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));

	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2DArray.ArraySize = 1;
	srvDesc.Texture2DArray.FirstArraySlice = 0u;
	srvDesc.Texture2DArray.MipLevels = 1u;
	srvDesc.Texture2DArray.MostDetailedMip = 0u;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

	hr = d3d->GetDevice()->CreateShaderResourceView(pDepthStencilTexture.Get(), &srvDesc, &pDepthSRV);
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

	srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));

	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2DArray.ArraySize = 1;
	srvDesc.Texture2DArray.FirstArraySlice = 0u;
	srvDesc.Texture2DArray.MipLevels = 1u;
	srvDesc.Texture2DArray.MostDetailedMip = 0u;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

	hr = d3d->GetDevice()->CreateShaderResourceView(texture.Get(), &srvDesc, &pNoMSDepthSRV);
	assert(SUCCEEDED(hr));

}

void Engine::Renderer::InitGBuffer(UINT wWidth, UINT wHeight)
{
	auto device = D3D::GetInstance()->GetDevice();
	Microsoft::WRL::ComPtr<ID3D11Texture2D> Albedo;

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = (UINT)wWidth;
	textureDesc.Height = (UINT)wHeight;
	textureDesc.MipLevels = 1u;
	textureDesc.ArraySize = 1u;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;

	HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &Albedo);
	assert(SUCCEEDED(hr));
	hr = device->CreateShaderResourceView(Albedo.Get(), nullptr, &m_GBuffer.Albedo);
	assert(SUCCEEDED(hr));
	hr = device->CreateRenderTargetView(Albedo.Get(), nullptr, &GBufferRTVs[0]);
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<ID3D11Texture2D> RoughMetal;
	textureDesc.Format = DXGI_FORMAT_R8G8_UNORM;
	hr = device->CreateTexture2D(&textureDesc, nullptr, &RoughMetal);
	assert(SUCCEEDED(hr));
	hr = device->CreateShaderResourceView(RoughMetal.Get(), nullptr, &m_GBuffer.RoughMetal);
	assert(SUCCEEDED(hr));
	hr = device->CreateRenderTargetView(RoughMetal.Get(), nullptr, &GBufferRTVs[1]);
	assert(SUCCEEDED(hr));


	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_SNORM;
	hr = device->CreateTexture2D(&textureDesc, nullptr, &m_GBuffer.normalsTexture);
	assert(SUCCEEDED(hr));
	hr = device->CreateShaderResourceView(m_GBuffer.normalsTexture.Get(), nullptr, &m_GBuffer.Normals);
	assert(SUCCEEDED(hr));
	D3D11_RENDER_TARGET_VIEW_DESC rtvNormalDesc;

	hr = device->CreateRenderTargetView(m_GBuffer.normalsTexture.Get(), nullptr, &GBufferRTVs[2]);
	assert(SUCCEEDED(hr));


	hr = device->CreateTexture2D(&textureDesc, nullptr, &m_GBuffer.secondNormalsTexture);
	assert(SUCCEEDED(hr));
	hr = device->CreateShaderResourceView(m_GBuffer.secondNormalsTexture.Get(), nullptr, &m_GBuffer.SecondNormals);
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<ID3D11Texture2D> Emmision;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	hr = device->CreateTexture2D(&textureDesc, nullptr, &Emmision);
	assert(SUCCEEDED(hr));
	hr = device->CreateShaderResourceView(Emmision.Get(), nullptr, &m_GBuffer.Emmision);
	assert(SUCCEEDED(hr));
	hr = device->CreateRenderTargetView(Emmision.Get(), nullptr, &GBufferRTVs[3]);
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<ID3D11Texture2D> ObjectID;
	textureDesc.Format = DXGI_FORMAT_R32_UINT;
	hr = device->CreateTexture2D(&textureDesc, nullptr, &ObjectID);
	assert(SUCCEEDED(hr));
	hr = device->CreateShaderResourceView(ObjectID.Get(), nullptr, &m_GBuffer.ObjectId);
	assert(SUCCEEDED(hr));
	hr = device->CreateRenderTargetView(ObjectID.Get(), nullptr, &GBufferRTVs[4]);
	assert(SUCCEEDED(hr));
}


void Engine::Renderer::updatePerFrameCB(float deltaTime, float wWidth, float wHeight, float farCLip, float nearClip)
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
	perFrameData.farClip = farCLip;
	perFrameData.nearClip = nearClip;

	perFrameBuffer.updateBuffer(&perFrameData);
}

void Engine::Renderer::CreateNoMSDepth()
{
	auto context = D3D::GetInstance()->GetContext();
	context->ClearDepthStencilView(pNoMSDepthStencil.Get(), D3D11_CLEAR_DEPTH, 0.0f, 0u);
	context->OMSetRenderTargets(0, nullptr, pNoMSDepthStencil.Get());
	context->OMSetDepthStencilState(pDSState.Get(), 0u);
	depthShader->BindShader();
	context->PSSetShaderResources(19u, 1u, pDepthSRV.GetAddressOf());
	context->Draw(3u, 0);
	ID3D11ShaderResourceView* const pSRVDepth = { NULL };
	context->PSSetShaderResources(19u, 1u, &pSRVDepth);
	context->OMSetRenderTargets(1u, pHDRRenderTarget.GetAddressOf(), pViewDepth.Get());
}

void Engine::Renderer::RenderParticles(Camera* camera)
{
	auto context = D3D::GetInstance()->GetContext();
	context->OMSetDepthStencilState(pDSReadOnlyState.Get(), 1u);
	context->OMSetBlendState(pBlendState.Get(), nullptr, 0xFFFFFFFF);

	context->PSSetShaderResources(23u, 1u, pNoMSDepthSRV.GetAddressOf());

	ParticleSystem::Init()->UpdateBuffers(camera->getPosition());
	ParticleSystem::Init()->Render();

	ID3D11ShaderResourceView* const pSRV = { NULL};
	context->PSSetShaderResources(23u, 1u, &pSRV);
}

void Engine::Renderer::FillGBuffer()
{
	m_GBuffer.Unbind();

	auto context = Engine::D3D::GetInstance()->GetContext();
	ID3D11RenderTargetView* views[5] = { GBufferRTVs[0].Get(),GBufferRTVs[1].Get(),GBufferRTVs[2].Get(),GBufferRTVs[3].Get(),GBufferRTVs[4].Get() };

	context->ClearDepthStencilView(pViewDepth.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0u);

	static const float color[] = { 0.5f, 0.5f,0.5f,1.0f };
	for (size_t i = 0; i < 5; i++)
	{
		context->ClearRenderTargetView(GBufferRTVs[i].Get(), color);
	}

	context->OMSetBlendState(NULL, NULL, 0xffffffff);
	context->OMSetRenderTargets(5U, views, pViewDepth.Get());
	MeshSystem::Init()->renderGBuffer(pDSState.Get());
	context->OMSetRenderTargets(0u, nullptr, nullptr);
}


void Engine::Renderer::Render(Camera* camera)
{
	auto context = Engine::D3D::GetInstance()->GetContext();

	static const float color[] = { 0.5f, 0.5f,0.5f,1.0f };
	context->ClearRenderTargetView(pHDRRenderTarget.Get(), color);

	Shadows(camera);

	PerViewCB perView = PerViewCB{ camera->getViewMatrix() * camera->getProjectionMatrix(), camera->getInverseViewMatrix(),
		camera->getInverseProjectionMatrix() * camera->getInverseViewMatrix(), camera->getInverseViewMatrix(),
		vec4(camera->getCameraFrustrum(Camera::LeftDown),.0f),
		vec4(camera->getCameraFrustrum(Camera::LeftUp) - camera->getCameraFrustrum(Camera::LeftDown),.0f),
		vec4(camera->getCameraFrustrum(Camera::RightDown) - camera->getCameraFrustrum(Camera::LeftDown),.0f),
		camera->getPosition()};

	
	perViewBuffer.updateBuffer(&perView);

	FillGBuffer();
	CreateNoMSDepth();
	RenderDecals();

	context->OMSetRenderTargets(1u, pHDRRenderTarget.GetAddressOf(), pViewDepth.Get());


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

	m_GBuffer.Bind(25u);
	context->PSSetShaderResources(30u, 1u, pNoMSDepthSRV.GetAddressOf());
	MeshSystem::Init()->defferedRender(pDSStencilOnlyState.Get());
	ID3D11ShaderResourceView* const nullSRV = { NULL };
	context->PSSetShaderResources(30u, 1u, &nullSRV);



	pSkyBox->BindSkyBox(2u);
	pSkyBox->Draw();


	context->OMSetDepthStencilState(pDSState.Get(), 0u);
	RenderParticles(camera);
	ID3D11ShaderResourceView* const pSRV[3] = { NULL, NULL, NULL };
	context->PSSetShaderResources(11, 3u, pSRV);
}

void Engine::Renderer::RenderDecals()
{
	auto context = Engine::D3D::GetInstance()->GetContext();

	context->CopyResource(m_GBuffer.secondNormalsTexture.Get(), m_GBuffer.normalsTexture.Get());

	ID3D11RenderTargetView* views[4] = { GBufferRTVs[0].Get(),GBufferRTVs[1].Get(),GBufferRTVs[2].Get(),GBufferRTVs[3].Get()};

	context->OMSetRenderTargets(4, views, pViewDepth.Get());
	context->OMSetDepthStencilState(pDSReadOnlyState.Get(), 0u);
	context->RSSetState(pCullBackRasterizerState.Get()); 
	//float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };  // Set blend factor (usually {1, 1, 1, 1} for standard blending)
	//UINT sampleMask = 0xFFFFFFFF;  // Sample mask (use all samples)
	//context->OMSetBlendState(pBlendState.Get(), blendFactor, sampleMask);
	context->PSSetShaderResources(25u, 1, m_GBuffer.ObjectId.GetAddressOf());
	context->PSSetShaderResources(26u, 1, m_GBuffer.SecondNormals.GetAddressOf());
	context->PSSetShaderResources(27u, 1, pNoMSDepthSRV.GetAddressOf());
	
	DecalSystem::Init()->UpdateBuffer();
	DecalSystem::Init()->Draw();

	context->OMSetRenderTargets(0u, nullptr, pViewDepth.Get());

	ID3D11ShaderResourceView* srvs[3] = { NULL,NULL,NULL };
	context->PSSetShaderResources(25, 3, srvs);

	///*
	//DecalSystem::Init()->UpdateBuffer();
	//DecalSystem::Init()->Draw();
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

	hr = D3D::GetInstance()->GetDevice()->CreateRasterizerState(&rasterDesc, &pCullBackRasterizerState);
	assert(SUCCEEDED(hr));

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend =
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend =
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp =
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	blendDesc.IndependentBlendEnable = TRUE;  // Enable independent blending for each render target

	// Enable blending for Albedo, Metalness, Roughness, Emission
	for (int i = 0; i < 4; i++) {
		if (i == 2)
			continue;

		blendDesc.RenderTarget[i].BlendEnable = TRUE;  // Enable blending
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	blendDesc.RenderTarget[2].BlendEnable = FALSE;  // No blending for normals
	blendDesc.RenderTarget[2].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;  // Still write to the buffer

	blendDesc.RenderTarget[4].BlendEnable = FALSE;  // No blending for normals
	blendDesc.RenderTarget[4].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;  // Still write to the buffer

	hr = D3D::GetInstance()->GetDevice()->CreateBlendState(&blendDesc, &pBlendState);
	assert(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};

	// Enable depth testing
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Disable depth writes
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;

	hr = D3D::GetInstance()->GetDevice()->CreateDepthStencilState(&depthStencilDesc, &pDSReadOnlyState);
	assert(SUCCEEDED(hr));

	depthShader = ShaderManager::CompileAndCreateShader("Depth", L"Shaders\\Depth\\DepthVS.hlsl", L"Shaders\\Depth\\DepthPS.hlsl", nullptr, nullptr);
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

	context->RSSetState(nullptr);

	ShadowSystem::Init()->BindShadowTextures(11u, 12u, 13u);
	ShadowSystem::Init()->BindShadowBuffers(5u, 6u);
}
