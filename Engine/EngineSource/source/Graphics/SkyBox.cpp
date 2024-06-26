#include "Graphics/SkyBox.h"
#include "Render/Camera.h"
#include "Graphics/TextureManager.h"
#include "Graphics/ShaderManager.h"



Engine::SkyBox::SkyBox(std::shared_ptr<Texture> skyBoxTexture, std::shared_ptr<shader> skyBoxShader) :
	m_skyBoxTexture(skyBoxTexture), m_skyBoxShader(skyBoxShader), m_pCamera(nullptr)
{
	m_cb.create();

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};

	// Enable depth testing
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Disable depth writes
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;


	D3D::GetInstance()->GetDevice()->CreateDepthStencilState(&depthStencilDesc, &m_readOnlyDepthBuffer);
}

Engine::SkyBox::SkyBox() : SkyBox(nullptr, nullptr) {}

void Engine::SkyBox::SetTexture(std::shared_ptr<Texture> text)
{
	m_skyBoxTexture = text;
}

void Engine::SkyBox::SetShader(std::shared_ptr<shader> shdr)
{
	m_skyBoxShader = shdr;
}

void Engine::SkyBox::BindCamera(const Camera* camera)
{
	m_pCamera = camera;
}

void Engine::SkyBox::BindSkyBox(UINT slot)
{
	if (m_pCamera)
	{
		cb_data.frustrum[0] = vec4(-1.0f, -1.0f, 1.0f, 0.0f) * m_pCamera->getInverseViewMatrix();
		cb_data.frustrum[1] = vec4(-1.0f, 3.0f, 1.0f, 0.0f) * m_pCamera->getInverseViewMatrix();
		cb_data.frustrum[2] = vec4(3.0f, -1.0f, 1.0f, 0.0f) * m_pCamera->getInverseViewMatrix();

		m_cb.updateBuffer(&cb_data);

		auto d3dContext = D3D::GetInstance()->GetContext();
		m_skyBoxShader->BindShader();
		m_skyBoxTexture->BindTexture(0u);
		m_cb.bind(slot, shaderTypes::VS);
	}
}

void Engine::SkyBox::Draw()
{
	auto d3dContext = D3D::GetInstance()->GetContext();
	d3dContext->OMSetDepthStencilState(m_readOnlyDepthBuffer.Get(), 1u);
	d3dContext->Draw(3, 0);
}
