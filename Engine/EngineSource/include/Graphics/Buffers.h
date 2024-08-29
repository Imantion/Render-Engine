#pragma once
#include "Graphics/D3D.h"
#include "Utils\EnumDefinitions.h"

namespace Engine
{
	template<typename T>
	class ConstBuffer
	{
	public:
		
		bool create(D3D11_USAGE usage = D3D11_USAGE_DYNAMIC, UINT amount = 1)
		{
			if (D3D* d3d = D3D::GetInstance())
			{
				D3D11_BUFFER_DESC cbd;
				cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	
				cbd.Usage = usage;
				cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				cbd.MiscFlags = 0u;
				cbd.ByteWidth = sizeof(T) * amount;
				cbd.StructureByteStride = 0u;

				HRESULT hr = d3d->GetDevice()->CreateBuffer(&cbd, nullptr, &m_constBuffer);
				assert(SUCCEEDED(hr));

				return true;
			}
			return false;

		}
		bool updateBuffer(const T* constBufferSource, UINT amount = 1u)
		{
			D3D* d3d = D3D::GetInstance();
			if (d3d && m_constBuffer.Get())
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				HRESULT hr = d3d->GetContext()->Map(m_constBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource);
				assert(SUCCEEDED(hr));
				T* data = (T*)mappedResource.pData;
				memcpy(data, constBufferSource, sizeof(T) * amount);
				d3d->GetContext()->Unmap(m_constBuffer.Get(), 0u);

				return true;
			}
			return false;
		}

		void bind(UINT slot, UINT typeOfShader)
		{
			auto context = D3D::GetInstance()->GetContext();
			if(typeOfShader & shaderTypes::VS)
				context->VSSetConstantBuffers(slot, 1, m_constBuffer.GetAddressOf());
			if(typeOfShader & shaderTypes::PS)
				context->PSSetConstantBuffers(slot, 1, m_constBuffer.GetAddressOf());
			if(typeOfShader & shaderTypes::HS)
				context->HSSetConstantBuffers(slot, 1, m_constBuffer.GetAddressOf());
			if(typeOfShader & shaderTypes::DS)
				context->DSSetConstantBuffers(slot, 1, m_constBuffer.GetAddressOf());
			if(typeOfShader & shaderTypes::GS)
				context->GSSetConstantBuffers(slot, 1, m_constBuffer.GetAddressOf());
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constBuffer;
	};

	template <typename T>
	class VertexBuffer
	{
	public:
		bool create(const T* data,UINT amountOfInstances, D3D11_USAGE usage = D3D11_USAGE_DEFAULT)
		{
			if(D3D* d3d = D3D::GetInstance())
			{
				D3D11_BUFFER_DESC bd;
				ZeroMemory(&bd, sizeof(bd));

				instances = amountOfInstances;

				bd.Usage = usage;
				bd.ByteWidth = sizeof(T) * instances;
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				D3D11_SUBRESOURCE_DATA sr_data;
				sr_data.pSysMem = data;

				HRESULT hr = d3d->GetDevice()->CreateBuffer(&bd, &sr_data, &m_vertexBuffer);
				assert(SUCCEEDED(hr));

				return true;
			}

			return false;
		}

		bool create(UINT amountOfInstances, D3D11_USAGE usage = D3D11_USAGE_DEFAULT)
		{
			if (amountOfInstances <= instances)
				return false;

			if (D3D* d3d = D3D::GetInstance())
			{
				instances = amountOfInstances;

				D3D11_BUFFER_DESC bd;
				ZeroMemory(&bd, sizeof(bd));
				bd.Usage = usage;
				bd.ByteWidth = sizeof(T) * instances;
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

				HRESULT hr = d3d->GetDevice()->CreateBuffer(&bd, nullptr, &m_vertexBuffer);
				assert(SUCCEEDED(hr));

				return true;
			}

			return false;
		}

		bool bind(UINT slot = 0u)
		{
			if (D3D* d3d = D3D::GetInstance())
			{
				UINT stride = sizeof(T);
				UINT offset = 0u;
				d3d->GetContext()->IASetVertexBuffers(slot, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
				return true;
			}
			return false;
		}

		void map(D3D11_MAPPED_SUBRESOURCE& mappedResource)
		{
			D3D* d3d = D3D::GetInstance();
			HRESULT hr = d3d->GetContext()->Map(m_vertexBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource);
		}

		void unmap()
		{
			D3D* d3d = D3D::GetInstance();
			d3d->GetContext()->Unmap(m_vertexBuffer.Get(), 0u);
		}

		UINT getSize() { return instances; }

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		UINT instances = 0u;
	};

	class IndexBuffer
	{
	public:
		bool create(unsigned int* data,UINT amountOfInstances)
		{
			if (D3D* d3d = D3D::GetInstance())
			{
				instances = amountOfInstances;
				D3D11_BUFFER_DESC bd;
				ZeroMemory(&bd, sizeof(bd));
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.ByteWidth = sizeof(unsigned int) * instances;
				bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
				bd.CPUAccessFlags = 0;
				bd.MiscFlags = 0;

				D3D11_SUBRESOURCE_DATA InitData;
				InitData.pSysMem = data;

				HRESULT hr = d3d->GetDevice()->CreateBuffer(&bd, &InitData, &m_indexBuffer);
				assert(SUCCEEDED(hr));
				return true;
			}
			
			return false;
		}

		bool bind()
		{
			if (D3D* d3d = D3D::GetInstance())
			{
				d3d->GetContext()->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
				return true;
			}
			return false;
		}

		UINT getSize()
		{
			return instances;
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
		UINT instances;
	};
}
