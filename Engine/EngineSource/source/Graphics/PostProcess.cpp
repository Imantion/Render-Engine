#include "Graphics/PostProcess.h"
#include "Graphics/ShaderManager.h"

Engine::PostProcess* Engine::PostProcess::m_instance;
std::mutex Engine::PostProcess::m_mutex;

Engine::PostProcess* Engine::PostProcess::Init()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_instance == nullptr)
    {
        m_instance = new PostProcess();
    }
    return m_instance;
}

void Engine::PostProcess::Deinit()
{
   delete m_instance;
   m_instance = nullptr;
}

Engine::PostProcess::PostProcess()
{
    m_constantBuffer.create();
    m_FXAAconstantBuffer.create();
}

void Engine::PostProcess::SetLightToColorShader(std::shared_ptr<shader> shader)
{
    m_lightToColorShader = shader;
}

void Engine::PostProcess::SetFXAAShader(std::shared_ptr<shader> shader)
{
    m_FXAAShader = shader;
}

void Engine::PostProcess::Resolve(ID3D11ShaderResourceView* srv, ID3D11RenderTargetView* dst)
{
    auto context = D3D::GetInstance()->GetContext();

    context->OMSetRenderTargets(1, &dst, nullptr);
    const FLOAT color[4] = { 0.5f,0.5f,0.5f,0.5f };
    context->ClearRenderTargetView(dst, color);
    context->OMSetBlendState(nullptr, nullptr, 0xfffff);
    m_lightToColorShader->BindInputLyout(nullptr);
    m_lightToColorShader->BindShader();

    m_constantBuffer.updateBuffer(&m_postProcessData);
    m_constantBuffer.bind(2u, PS);
    context->PSSetShaderResources(0, 1, &srv);

    context->Draw(3u, 0);

    ID3D11ShaderResourceView* const pSRV[1] = { NULL };
    context->PSSetShaderResources(0, 1, pSRV);

}

void Engine::PostProcess::FXAA(ID3D11ShaderResourceView* srv, ID3D11RenderTargetView* dst)
{
    auto context = D3D::GetInstance()->GetContext();

    context->OMSetRenderTargets(1, &dst, nullptr);
    m_FXAAShader->BindInputLyout(nullptr);
    m_FXAAShader->BindShader();

    m_FXAAconstantBuffer.updateBuffer(&m_FXAAData);
    m_FXAAconstantBuffer.bind(2u, PS);
    context->PSSetShaderResources(0, 1, &srv);

    context->Draw(3u, 0);

    ID3D11ShaderResourceView* const pSRV[1] = { NULL };
    context->PSSetShaderResources(0, 1, pSRV);
}

void Engine::PostProcess::AddEV100(float EV100)
{
    m_postProcessData.EV100 += EV100;
}

void Engine::PostProcess::SetGamma(float gamma)
{
    m_postProcessData.gamma = gamma;
}

void Engine::PostProcess::UpdateFXAABuffer(float qualitySubpix, float qualityEdgeThreshold, float qualityEdgeThresholdMin)
{
    m_FXAAData.qualitySubpix = qualitySubpix;
    m_FXAAData.qualityEdgeThreshold = qualityEdgeThreshold;
    m_FXAAData.qualityEdgeThresholdMin = qualityEdgeThresholdMin;
}

void Engine::PostProcess::SetRTSize(UINT width, UINT height)
{
    m_renderTargetWidth = width;
    m_renderTargetHeight = height;

    m_FXAAData.imageSize[0] = (float)width;
    m_FXAAData.imageSize[1] = (float)height;
    m_FXAAData.imageSize[2] = 1.0f / (float)width;
    m_FXAAData.imageSize[3] = 1.0f / (float)height;
}


