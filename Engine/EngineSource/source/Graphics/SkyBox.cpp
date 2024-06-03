#include "Graphics/SkyBox.h"
#include "Render/Camera.h"
#include "Graphics/TextureManager.h"
#include "Graphics/ShaderManager.h"

Engine::SkyBox::SkyBox(Texture* skyBoxTexture, shader* skyBoxShader) :
	m_skyBoxTexture(skyBoxTexture), m_skyBoxShader(skyBoxShader), m_pCamera(nullptr)
{
	m_cb.create();

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};

	// Enable depth testing
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Disable depth writes
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL; // Use the standard less-than depth test

	// Stencil operations (if needed, you can customize this part)
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	D3D::GetInstance()->GetDevice()->CreateDepthStencilState(&depthStencilDesc, &m_readOnlyDepthBuffer);
}

void Engine::SkyBox::BindCamera(const Camera* camera)
{
	m_pCamera = camera;
}

void Engine::SkyBox::BindSkyBox(UINT slot)
{
	if (m_pCamera)
	{
		cb_data.frustrums[0] = vec4(m_pCamera->calculateRayDirection(vec2(-1.0f, -1.0f)),0.0f);
		cb_data.frustrums[1] = vec4(m_pCamera->calculateRayDirection(vec2(-1.0f, 1.0f)),0.0f);
		cb_data.frustrums[2] = vec4(m_pCamera->calculateRayDirection(vec2(1.0f, -1.0f)),0.0f);
		cb_data.frustrums[3] = vec4(m_pCamera->calculateRayDirection(vec2(1.0f, 1.0f)),0.0f);

		//cb_data.frustrums[0] = vec4(-1, -1, 1, 0);
		//cb_data.frustrums[1] = vec4(-1, 1, 1, 0);
		//cb_data.frustrums[2] = vec4(1, -1, 1, 0);

		m_cb.updateBuffer(&cb_data);

		
		auto d3dContext = D3D::GetInstance()->GetContext();
		m_skyBoxTexture->BindTexture(0u);
		m_skyBoxShader->BindShader();
		d3dContext->VSSetConstantBuffers(2u, 1u, m_cb.m_constBuffer.GetAddressOf());
		d3dContext->OMSetDepthStencilState(m_readOnlyDepthBuffer.Get(), 1u);
	}
}
