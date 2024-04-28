#include "Graphics/D3D.h"
#include <assert.h>

Engine::D3D* Engine::D3D::pInstance;
std::mutex Engine::D3D::mutex_;


Engine::D3D* Engine::D3D::Init(UINT flags)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!pInstance)
    {
        pInstance = new D3D(flags);
    }

    return pInstance;
}

Engine::D3D::D3D(UINT flags)
{
    #ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    HRESULT hr = D3D11CreateDevice(
        nullptr, 
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr, 
        flags, 
        nullptr, 
        0, 
        D3D11_SDK_VERSION, 
        &pDevice, 
        nullptr, 
        &pContext);

    assert(SUCCEEDED(hr));
}

void Engine::D3D::CreateVertexShaderAndInputLayout(const wchar_t* source,const D3D11_INPUT_ELEMENT_DESC* ied, UINT iedSize)
{
    Microsoft::WRL::ComPtr<ID3DBlob> VS;

    HRESULT hr = D3DCompileFromFile(
        source, NULL, NULL, "main", "vs_5_0", 0, 0, &VS, 0);
    assert(SUCCEEDED(hr));
    
    hr = pDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), nullptr, &pVertexShader);
    assert(SUCCEEDED(hr));

    pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u);

    hr = pDevice->CreateInputLayout(ied, iedSize, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
    assert(SUCCEEDED(hr));

    pContext->IASetInputLayout(pLayout.Get());
}

void Engine::D3D::CreatePixelShader(const wchar_t* source)
{
    Microsoft::WRL::ComPtr<ID3DBlob> PS;

    HRESULT hr = D3DCompileFromFile(
        source, NULL, NULL, "main", "ps_5_0", 0, 0, &PS, 0);
    assert(SUCCEEDED(hr));

    hr = pDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), nullptr, &pPixelShader);
    assert(SUCCEEDED(hr));

    pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);
}

void Engine::D3D::CreateBuffer(const D3D11_BUFFER_DESC* bd, const D3D11_SUBRESOURCE_DATA* sd)
{
    HRESULT hr = pDevice->CreateBuffer(bd, sd, &pBuffer);
    assert(SUCCEEDED(hr));
}

void Engine::D3D::Reset()
{
    delete pInstance;
    pInstance = nullptr;
}