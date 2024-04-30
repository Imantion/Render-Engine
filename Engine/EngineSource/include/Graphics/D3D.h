#pragma once
#include <d3d11.h>
#include <d3d10.h>
#include <d3dcompiler.h>
#include <mutex>
#include <assert.h>
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

		ID3D11Device* GetDevice() { return pDevice.Get(); }
		ID3D11Device** GetDeviceAdress() { return pDevice.GetAddressOf(); }

		ID3D11DeviceContext* GetContext() { return pContext.Get(); }
		ID3D11DeviceContext** GetContextAdress() { return pContext.GetAddressOf(); }
	private:
		static D3D* pInstance;
		static std::mutex mutex_;
		Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;

	protected:
		D3D(UINT flags);
	
	public:
		D3D(D3D& other) = delete;
		void operator=(const D3D&) = delete;
	};

	template<typename T>
	class ConstBuffer
	{
	public:
		bool updateBuffer(const T* constBufferSource)
		{
			D3D* d3d = D3D::GetInstance();
			if (d3d && m_constBuffer.Get())
			{
				constBufferData = *constBufferSource;
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				HRESULT hr = d3d->GetContext()->Map(m_constBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource);
				assert(SUCCEEDED(hr));
				T* data = (T*)mappedResource.pData;
				memcpy(data, constBufferSource, sizeof(*constBufferSource));
				d3d->GetContext()->Unmap(m_constBuffer.Get(), 0u);

				return true;
			}
			return false;
		}

		bool updateBuffer()
		{
			D3D* d3d = D3D::GetInstance();
			if (d3d && m_constBuffer.Get())
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				HRESULT hr = d3d->GetContext()->Map(m_constBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource);
				assert(SUCCEEDED(hr));
				T* data = (T*)mappedResource.pData;
				memcpy(data, &constBufferData, sizeof(constBufferData));
				d3d->GetContext()->Unmap(m_constBuffer.Get(), 0u);

				return true;
			}
			return false;
		}
	public:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constBuffer;
		T constBufferData;
	};
}

