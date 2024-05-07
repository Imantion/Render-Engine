#include "Graphics/Renderer.h"
#include "Graphics/Model.h"

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
	if (D3D* d3d = D3D::GetInstance())
	{
		HRESULT hr = d3d->GetDevice()->CreateRenderTargetView(RenderBuffer, nullptr, &pRenderTarget);
		assert(SUCCEEDED(hr));
		InitDepth(wWidth, wHeight);

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

void Engine::Renderer::Render()
{
	Engine::D3D* d3d = Engine::D3D::GetInstance();

	d3d->GetContext()->OMSetRenderTargets(1u, pRenderTarget.GetAddressOf(), pViewDepth.Get());

	const float color[] = { 0.5f, 0.5f,0.5f,1.0f };
	
	d3d->GetContext()->ClearRenderTargetView(pRenderTarget.Get(), color);
	d3d->GetContext()->ClearDepthStencilView(pViewDepth.Get(), D3D11_CLEAR_DEPTH, 0.0f, 0u);
}

Engine::Renderer::Renderer()
{
    perFrameBuffer.create();
    perViewBuffer.create();
}
