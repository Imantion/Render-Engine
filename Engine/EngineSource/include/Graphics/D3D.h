#pragma once
#include <d3d11.h>
#include <d3d10.h>
#include <d3dcompiler.h>
#include <mutex>
#include <wrl.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")
#pragma comment(lib, "dxgi.lib")

namespace Engine
{
	struct Vertex
	{
		FLOAT x, y;
		FLOAT color[4];
	};

	class D3D
	{
	public:
		static D3D* Init(UINT flags = 0u);
		static D3D* GetInstance() { return pInstance; }

		void Reset();

		void CreateBuffer(const D3D11_BUFFER_DESC* bd, const D3D11_SUBRESOURCE_DATA* sd);
		ID3D11Device* GetDevice() { return pDevice.Get(); }
		ID3D11Device** GetDeviceAdress() { return pDevice.GetAddressOf(); }

		ID3D11DeviceContext* GetContext() { return pContext.Get(); }
		ID3D11DeviceContext** GetContextAdress() { return pContext.GetAddressOf(); }

		ID3D11Buffer** GetBufferAdress() { return pBuffer.GetAddressOf(); }
	private:
		static D3D* pInstance;
		static std::mutex mutex_;
		Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer;

	protected:
		D3D(UINT flags);
	
	public:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> pLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstBuffer;
		D3D(D3D& other) = delete;
		void operator=(const D3D&) = delete;
	};
}

