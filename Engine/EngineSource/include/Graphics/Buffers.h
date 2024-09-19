#pragma once
#include "Graphics/D3D.h"
#include "Utils\Definitions.h"
#include "DirectXMath.h"

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
			if (typeOfShader & shaderTypes::CS)
				context->CSSetConstantBuffers(slot, 1, m_constBuffer.GetAddressOf());
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


	template<typename T>
	class StructuredBuffer
	{
	public:

		void create(const T* data, UINT amount, bool writeMode = false, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC)
		{
			auto d3d = D3D::GetInstance();

			UINT FLAGS = D3D11_BIND_SHADER_RESOURCE;
			if (writeMode)
			{
				FLAGS |= D3D11_BIND_UNORDERED_ACCESS;
				usage = D3D11_USAGE_DEFAULT;
			}

			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.BindFlags = FLAGS;
			bufferDesc.Usage = usage;
			bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.ByteWidth = sizeof(T) * amount;
			bufferDesc.StructureByteStride = sizeof(T);

			D3D11_SUBRESOURCE_DATA initData = {};
			initData.pSysMem = data;

			HRESULT hr = d3d->GetDevice()->CreateBuffer(&bufferDesc, data ? &initData : nullptr, &m_buffer);
			assert(SUCCEEDED(hr));

			// Describe the Shader Resource View (SRV)
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.NumElements = amount; // Number of elements

			// Create the SRV
			hr = d3d->GetDevice()->CreateShaderResourceView(m_buffer.Get(), &srvDesc, &m_structuredBufferSRV);
			assert(SUCCEEDED(hr));

			if (writeMode)
			{
				D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = DXGI_FORMAT_UNKNOWN;
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				uavDesc.Buffer.NumElements = amount; // Number of elements

				// Create the SRV
				hr = d3d->GetDevice()->CreateUnorderedAccessView(m_buffer.Get(), &uavDesc, m_UAV.GetAddressOf());
				assert(SUCCEEDED(hr));

				canWrite = writeMode;
			}
		}

		void create(UINT amount, bool writeMode = false, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC)
		{
			create(nullptr, amount, writeMode, usage);
		}

		bool updateBuffer(const T* BufferSource, UINT amount = 1u)
		{
			if (canWrite)
				return false;

			D3D* d3d = D3D::GetInstance();
			if (d3d && m_buffer.Get())
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				HRESULT hr = d3d->GetContext()->Map(m_buffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource);
				assert(SUCCEEDED(hr));
				T* data = (T*)mappedResource.pData;
				memcpy(data, BufferSource, sizeof(T) * amount);
				d3d->GetContext()->Unmap(m_buffer.Get(), 0u);

				return true;
			}
			return false;
		}

		void bind(UINT slot, UINT typeOfShader)
		{
			auto context = D3D::GetInstance()->GetContext();
			if (typeOfShader & shaderTypes::VS)
				context->VSSetShaderResources(slot, 1, m_structuredBufferSRV.GetAddressOf());
			if (typeOfShader & shaderTypes::PS)
				context->PSSetShaderResources(slot, 1, m_structuredBufferSRV.GetAddressOf());
			if (typeOfShader & shaderTypes::HS)
				context->HSSetShaderResources(slot, 1, m_structuredBufferSRV.GetAddressOf());
			if (typeOfShader & shaderTypes::DS)
				context->DSSetShaderResources(slot, 1, m_structuredBufferSRV.GetAddressOf());
			if (typeOfShader & shaderTypes::GS)
				context->GSSetShaderResources(slot, 1, m_structuredBufferSRV.GetAddressOf());
		}

		void unbind(UINT slot, UINT typeOfShader)
		{
			auto context = D3D::GetInstance()->GetContext();
			ID3D11ShaderResourceView* nullSRV[1] = { NULL };
			if (typeOfShader & shaderTypes::VS)
				context->VSSetShaderResources(slot, 1, nullSRV);
			if (typeOfShader & shaderTypes::PS)
				context->PSSetShaderResources(slot, 1, nullSRV);
			if (typeOfShader & shaderTypes::HS)
				context->HSSetShaderResources(slot, 1, nullSRV);
			if (typeOfShader & shaderTypes::DS)
				context->DSSetShaderResources(slot, 1, nullSRV);
			if (typeOfShader & shaderTypes::GS)
				context->GSSetShaderResources(slot, 1, nullSRV);
		}

		void bindWrite(UINT slot)
		{
			auto context = D3D::GetInstance()->GetContext();

			if (canWrite)
				context->CSSetUnorderedAccessViews(slot, 1u, m_UAV.GetAddressOf(), nullptr);
		}

		void unbindWrite(UINT slot)
		{
			auto context = D3D::GetInstance()->GetContext();

			if (canWrite)
			{
				ID3D11UnorderedAccessView* nullUAV = nullptr;
				context->CSSetUnorderedAccessViews(slot, 1u, &nullUAV, nullptr);
			}
		}

		ID3D11UnorderedAccessView* getUAV()
		{
			return m_UAV.Get();
		}

		ID3D11Buffer* getBuffer()
		{
			return m_buffer.Get();
		}


	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_structuredBufferSRV;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_UAV;
		bool canWrite = false;
	};

	template<typename T>
	class Buffer
	{
	public:

		void create(const T* data, UINT amount, DXGI_FORMAT format, UINT MiscFlags = 0, bool writeMode = false, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC)
		{
			auto d3d = D3D::GetInstance();

			UINT FLAGS = D3D11_BIND_SHADER_RESOURCE;
			UINT CPUFLAGS = D3D11_CPU_ACCESS_WRITE;
			if (writeMode)
			{
				FLAGS |= D3D11_BIND_UNORDERED_ACCESS;
				usage = D3D11_USAGE_DEFAULT;
				CPUFLAGS = 0;
			}

			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.BindFlags = FLAGS;
			bufferDesc.Usage = usage;
			bufferDesc.MiscFlags = 0;
			bufferDesc.CPUAccessFlags = CPUFLAGS;
			bufferDesc.ByteWidth = sizeof(T) * amount;
			bufferDesc.MiscFlags = MiscFlags;

			D3D11_SUBRESOURCE_DATA initData = {};
			initData.pSysMem = data;

			HRESULT hr = d3d->GetDevice()->CreateBuffer(&bufferDesc, data ? &initData : nullptr, &m_buffer);
			assert(SUCCEEDED(hr));

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.NumElements = sizeof(T) / 4; // Number of elements

			// Create the SRV
			hr = d3d->GetDevice()->CreateShaderResourceView(m_buffer.Get(), &srvDesc, &m_bufferSRV);
			assert(SUCCEEDED(hr));

			if (writeMode)
			{
				D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = format;
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				uavDesc.Buffer.NumElements = sizeof(T) / 4; // Number of elements

				// Create the SRV
				hr = d3d->GetDevice()->CreateUnorderedAccessView(m_buffer.Get(), &uavDesc, &m_UAV);
				assert(SUCCEEDED(hr));

				canWrite = writeMode;
			}
		}

		void create(UINT amount, DXGI_FORMAT format, UINT MiscFlags = 0, bool writeMode = false, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC)
		{
			create(nullptr, amount, format, MiscFlags, writeMode, usage);
		}

		bool updateBuffer(const T* BufferSource, UINT amount = 1u)
		{
			if (canWrite)
				return false;

			D3D* d3d = D3D::GetInstance();
			if (d3d && m_buffer.Get())
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				HRESULT hr = d3d->GetContext()->Map(m_buffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource);
				assert(SUCCEEDED(hr));
				T* data = (T*)mappedResource.pData;
				memcpy(data, BufferSource, sizeof(T) * amount);
				d3d->GetContext()->Unmap(m_buffer.Get(), 0u);

				return true;
			}
			return false;
		}

		void bind(UINT slot, UINT typeOfShader)
		{
			auto context = D3D::GetInstance()->GetContext();
			if (typeOfShader & shaderTypes::VS)
				context->VSSetShaderResources(slot, 1, m_bufferSRV.GetAddressOf());
			if (typeOfShader & shaderTypes::PS)
				context->PSSetShaderResources(slot, 1, m_bufferSRV.GetAddressOf());
			if (typeOfShader & shaderTypes::HS)
				context->HSSetShaderResources(slot, 1, m_bufferSRV.GetAddressOf());
			if (typeOfShader & shaderTypes::DS)
				context->DSSetShaderResources(slot, 1, m_bufferSRV.GetAddressOf());
			if (typeOfShader & shaderTypes::GS)
				context->GSSetShaderResources(slot, 1, m_bufferSRV.GetAddressOf());
		}

		void unbind(UINT slot, UINT typeOfShader)
		{
			auto context = D3D::GetInstance()->GetContext();
			ID3D11ShaderResourceView* nullSRV[1] = { NULL };
			if (typeOfShader & shaderTypes::VS)
				context->VSSetShaderResources(slot, 1, nullSRV);
			if (typeOfShader & shaderTypes::PS)
				context->PSSetShaderResources(slot, 1, nullSRV);
			if (typeOfShader & shaderTypes::HS)
				context->HSSetShaderResources(slot, 1, nullSRV);
			if (typeOfShader & shaderTypes::DS)
				context->DSSetShaderResources(slot, 1, nullSRV);
			if (typeOfShader & shaderTypes::GS)
				context->GSSetShaderResources(slot, 1, nullSRV);
		}

		void unbindWrite(UINT slot)
		{
			auto context = D3D::GetInstance()->GetContext();

			if (canWrite)
			{
				ID3D11UnorderedAccessView* nullUAV = nullptr;
				context->CSSetUnorderedAccessViews(slot, 1u, &nullUAV, nullptr);
			}
		}

		void bindWrite(UINT slot)
		{
			auto context = D3D::GetInstance()->GetContext();

			if (canWrite)
				context->CSSetUnorderedAccessViews(slot, 1u, m_UAV.GetAddressOf(), nullptr);
		}

		ID3D11UnorderedAccessView* getUAV()
		{
			return m_UAV.Get();
		}

		ID3D11Buffer* getBuffer()
		{
			return m_buffer.Get();
		}

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bufferSRV;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_UAV;
		bool canWrite = false;
	};


	class ByteAddressBuffer
	{
	public:

		void create(UINT* data, UINT ByteWidth, bool writeMode = false, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC)
		{
			auto d3d = D3D::GetInstance();

			UINT FLAGS = D3D11_BIND_SHADER_RESOURCE;
			if (writeMode)
			{
				FLAGS |= D3D11_BIND_UNORDERED_ACCESS;
				usage = D3D11_USAGE_DEFAULT;
			}

			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage = D3D11_USAGE_DEFAULT; // or D3D11_USAGE_DYNAMIC depending on your needs
			bufferDesc.ByteWidth = ByteWidth; // total size of the buffer in bytes
			bufferDesc.BindFlags = FLAGS; // Bind as a Shader Resource
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // or D3D11_CPU_ACCESS_WRITE if you want to map the buffer later
			bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
			bufferDesc.StructureByteStride = 0; // Stride is 0 for ByteAddressBuffer


			D3D11_SUBRESOURCE_DATA initData = {};
			initData.pSysMem = data;

			HRESULT hr = d3d->GetDevice()->CreateBuffer(&bufferDesc, data ? &initData : nullptr, &m_byteAddressBuffer);
			assert(SUCCEEDED(hr));


			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			srvDesc.BufferEx.FirstElement = 0;
			srvDesc.BufferEx.NumElements = ByteWidth / 4; // Number of 4-byte elements

			// Create the SRV
			hr = d3d->GetDevice()->CreateShaderResourceView(m_byteAddressBuffer.Get(), &srvDesc, &m_byteAddressBufferSRV);
			assert(SUCCEEDED(hr));

			if (writeMode)
			{
				canWrite = writeMode;

				D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
				uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				uavDesc.Format = DXGI_FORMAT_R32_TYPELESS; // For RWByteAddressBuffer
				uavDesc.Buffer.FirstElement = 0;
				uavDesc.Buffer.NumElements = ByteWidth / 4; // Number of 4-byte elements
				uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW; // For RWByteAddressBuffer

				hr = d3d->GetDevice()->CreateUnorderedAccessView(m_byteAddressBuffer.Get(), &uavDesc, &m_UAV);
				assert(SUCCEEDED(hr));
			}
		}

		void create(UINT amount, bool writeMode = false, D3D11_USAGE usage = D3D11_USAGE_DYNAMIC)
		{
			create(nullptr, amount, writeMode, usage);
		}

		bool updateBuffer(const void* BufferSource, UINT dataSize)
		{
			if (canWrite)
				return false;

			D3D* d3d = D3D::GetInstance();
			if (d3d && m_byteAddressBuffer.Get())
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				HRESULT hr = d3d->GetContext()->Map(m_byteAddressBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResource);
				assert(SUCCEEDED(hr));
				void* data = mappedResource.pData;
				memcpy(data, BufferSource, dataSize);
				d3d->GetContext()->Unmap(m_byteAddressBuffer.Get(), 0u);

				return true;
			}
			return false;
		}

		void bind(UINT slot, UINT typeOfShader)
		{
			auto context = D3D::GetInstance()->GetContext();

			if (typeOfShader & shaderTypes::VS)
				context->VSSetShaderResources(slot, 1, m_byteAddressBufferSRV.GetAddressOf());
			if (typeOfShader & shaderTypes::PS)
				context->PSSetShaderResources(slot, 1, m_byteAddressBufferSRV.GetAddressOf());
			if (typeOfShader & shaderTypes::HS)
				context->HSSetShaderResources(slot, 1, m_byteAddressBufferSRV.GetAddressOf());
			if (typeOfShader & shaderTypes::DS)
				context->DSSetShaderResources(slot, 1, m_byteAddressBufferSRV.GetAddressOf());
			if (typeOfShader & shaderTypes::GS)
				context->GSSetShaderResources(slot, 1, m_byteAddressBufferSRV.GetAddressOf());
		}

		void unbind(UINT slot, UINT typeOfShader)
		{
			auto context = D3D::GetInstance()->GetContext();
			ID3D11ShaderResourceView* nullSRV[1] = { NULL };
			if (typeOfShader & shaderTypes::VS)
				context->VSSetShaderResources(slot, 1, nullSRV);
			if (typeOfShader & shaderTypes::PS)
				context->PSSetShaderResources(slot, 1, nullSRV);
			if (typeOfShader & shaderTypes::HS)
				context->HSSetShaderResources(slot, 1, nullSRV);
			if (typeOfShader & shaderTypes::DS)
				context->DSSetShaderResources(slot, 1, nullSRV);
			if (typeOfShader & shaderTypes::GS)
				context->GSSetShaderResources(slot, 1, nullSRV);
		}

		void unbindWrite(UINT slot)
		{
			auto context = D3D::GetInstance()->GetContext();

			if (canWrite)
			{
				ID3D11UnorderedAccessView* nullUAV = nullptr;
				context->CSSetUnorderedAccessViews(slot, 1u, &nullUAV, nullptr);
			}
		}

		void bindWrite(UINT slot)
		{
			auto context = D3D::GetInstance()->GetContext();

			if(canWrite)
			context->CSSetUnorderedAccessViews(slot, 1u, m_UAV.GetAddressOf(), nullptr);
		}

		ID3D11UnorderedAccessView* getUAV()
		{
			return m_UAV.Get();
		}

		ID3D11Buffer* getBuffer()
		{
			return m_byteAddressBuffer.Get();
		}


	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_byteAddressBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_byteAddressBufferSRV;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_UAV;
		bool canWrite = false;
	};
}
