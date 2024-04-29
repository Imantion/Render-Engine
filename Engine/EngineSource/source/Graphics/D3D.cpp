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