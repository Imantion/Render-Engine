#pragma once
#include <d3d11.h>
#include <d3d10.h>
#include <d3dcompiler.h>
#include <dxgi1_5.h>
#include <mutex>
#include <assert.h>
#include <wrl.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")
#pragma comment(lib, "dxgi.lib")

namespace Engine
{
	class D3D
	{
	public:
		static D3D* Init(UINT flags = 0u);
		static D3D* GetInstance() { return pInstance; }

		void Reset();

		ID3D11Device* GetDevice() { return pDevice.Get(); }
		ID3D11Device** GetDeviceAdress() { return pDevice.GetAddressOf(); }

		ID3D11DeviceContext* GetContext() { return pContext.Get(); }
		ID3D11DeviceContext** GetContextAdress() { return pContext.GetAddressOf(); }

		IDXGIFactory5* GetFactory() { return pFactory.Get(); }
		IDXGIFactory5** GetFactoryAdress() { return pFactory.GetAddressOf(); }
	private:
		static D3D* pInstance;
		static std::mutex mutex_;
		Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
		Microsoft::WRL::ComPtr<IDXGIFactory5> pFactory;

	protected:
		D3D(UINT flags);
	
	public:
		D3D(D3D& other) = delete;
		void operator=(const D3D&) = delete;
	};
}

