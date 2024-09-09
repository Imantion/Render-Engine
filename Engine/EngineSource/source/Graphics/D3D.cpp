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

    D3D_FEATURE_LEVEL featureLevels[] = 
    {
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_9_3,
    D3D_FEATURE_LEVEL_9_2,
    D3D_FEATURE_LEVEL_9_1,
};
	D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(
        nullptr, 
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr, 
        flags, 
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION, 
        &pDevice, 
        &featureLevel,
        &pContext);
    assert(SUCCEEDED(hr));
    hr = CreateDXGIFactory2(0, __uuidof(IDXGIFactory5), &pFactory);
    assert(SUCCEEDED(hr));
}



void Engine::D3D::Reset()
{
    delete pInstance;
    pInstance = nullptr;
}