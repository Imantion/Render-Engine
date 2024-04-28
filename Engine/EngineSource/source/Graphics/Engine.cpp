#include "Graphics/Engine.h"
#include "Window/Window.h"
#include <assert.h>


bool Engine::Engine::isInitialized = false;

void Engine::Engine::Init()
{
	D3D::Init();
	isInitialized = true;
}

void Engine::Engine::Deinit()
{
	D3D::GetInstance()->Reset();
}

void Engine::Engine::PrepareTriangle()
{
	

	if (D3D* d3d = D3D::GetInstance())
	{
		D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		d3d->CreatePixelShader(L"PixelShader.hlsl");
		d3d->CreateVertexShaderAndInputLayout(L"VertexShader.hlsl", ied, 2u);

		Vertex OutVertices[] =
		{
			{0.0f, 0.5f,  {1.0f, 0.0f, 0.0f, 1.0f}},
			{0.45f, -0.5, {0.0f, 1.0f, 0.0f, 1.0f}},
			{-0.45f, -0.5f,{0.0f, 0.0f, 1.0f, 1.0f}}
		};

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(Vertex) * 3;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA sr_data;
		sr_data.pSysMem = OutVertices;

		d3d->CreateBuffer(&bd, &sr_data);

		ConstantBuffer cb = { {800.0f,400.0f, 0.0f, 0.0f}, 0.0f };

		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(cb);
		cbd.StructureByteStride = 0u;
		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &cb;
		HRESULT hr = d3d->GetDevice()->CreateBuffer(&cbd, &csd, &d3d->pConstBuffer);
		assert(SUCCEEDED(hr));

		d3d->GetContext()->PSSetConstantBuffers(0u, 1u, d3d->pConstBuffer.GetAddressOf());

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		d3d->GetContext()->IASetVertexBuffers(0, 1, d3d->GetBufferAdress(), &stride, &offset);
		d3d->GetContext()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

void Engine::Engine::PrepareCurlesque()
{
	if (D3D* d3d = D3D::GetInstance())
	{
		D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		d3d->CreateVertexShaderAndInputLayout(L"VertexShader.hlsl", ied, 2u);
		d3d->CreatePixelShader(L"PixelShader.hlsl");

		Vertex OutVertices[] =
		{
			{-1.0f, -1.0f,  {1.0f, 0.0f, 0.0f, 1.0f}},
			{-1.0f, 1.0f, {0.0f, 1.0f, 0.0f, 1.0f}},
			{1.0f, -1.0f,{0.0f, 0.0f, 1.0f, 1.0f}},
			{1.0f, 1.0f,{0.0f, 0.0f, 1.0f, 1.0f}},
		};

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(Vertex) * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA sr_data;
		sr_data.pSysMem = OutVertices;

		d3d->CreateBuffer(&bd, &sr_data);
		d3d->GetContext()->PSSetConstantBuffers(0u, 1u, d3d->pConstBuffer.GetAddressOf());

		
		ConstantBuffer cb = { {800.0f,400.0f, 0.0f, 0.0f}, 0.0f};

		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(cb);
		cbd.StructureByteStride = 0u;
		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &cb;
		HRESULT hr = d3d->GetDevice()->CreateBuffer(&cbd, &csd, &d3d->pConstBuffer);
		assert(SUCCEEDED(hr));

		d3d->GetContext()->PSSetConstantBuffers(0u, 1u, d3d->pConstBuffer.GetAddressOf());

		
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		d3d->GetContext()->IASetVertexBuffers(0u, 1u, d3d->GetBufferAdress(), &stride, &offset);
		d3d->GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	}
}
