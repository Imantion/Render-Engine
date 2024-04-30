#include "D3DApplication.h"
#include "Graphics/Engine.h"
#include "Graphics/ShaderManager.h"
#include <assert.h>

D3DApplication::D3DApplication(int windowWidth, int windowHeight, WinProc windowEvent)
{
	pWindow.reset(new Engine::Window(windowWidth, windowHeight, windowEvent));
	PSConstBuffer.constBufferData.g_resolution[0] = (FLOAT)windowWidth;
	PSConstBuffer.constBufferData.g_resolution[1] = (FLOAT)windowHeight;
	PSConstBuffer.constBufferData.g_resolution[2] = PSConstBuffer.constBufferData.g_resolution[3] = 1 / ((FLOAT)windowWidth * (FLOAT)windowHeight);

}

void D3DApplication::PrepareTriangle()
{
	if (Engine::D3D* d3d = Engine::D3D::GetInstance())
	{
		D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		D3D_SHADER_MACRO psMacro[] = { "FIRST_SHADER", "0", NULL, NULL };

		Engine::shader* triangleShader = Engine::ShaderManager::CompileAndCreateShader("Triangle", L"Shaders/VertexShader.hlsl", L"Shaders/PixelShader.hlsl", nullptr, psMacro);
		if (!triangleShader)
			throw std::runtime_error("Failed to compile and create shader!");


		d3d->GetContext()->VSSetShader(triangleShader->vertexShader.Get(), nullptr, 0u);

		d3d->GetDevice()->CreateInputLayout(ied, 2u, triangleShader->vertexShaderBlob->GetBufferPointer(), triangleShader->vertexShaderBlob->GetBufferSize(), &pLayout);
		d3d->GetContext()->IASetInputLayout(pLayout.Get());

		d3d->GetContext()->PSSetShader(triangleShader->pixelShader.Get(), nullptr, 0u);

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

		d3d->GetDevice()->CreateBuffer(&bd, &sr_data, &pBuffer);
		
		ConstantBuffer cb = {{800.0f,400.0f, 0.0f, 0.0f}, 0.0f };

		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(cb);
		cbd.StructureByteStride = 0u;
		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &cb;
		HRESULT hr = d3d->GetDevice()->CreateBuffer(&cbd, &csd, &PSConstBuffer.m_constBuffer);
		assert(SUCCEEDED(hr));

		d3d->GetContext()->PSSetConstantBuffers(0u, 1u, PSConstBuffer.m_constBuffer.GetAddressOf());

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		d3d->GetContext()->IASetVertexBuffers(0, 1, pBuffer.GetAddressOf(), &stride, &offset);
		d3d->GetContext()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

void D3DApplication::PrepareCurlesque()
{
	if (Engine::D3D* d3d = Engine::D3D::GetInstance())
	{
		D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		D3D_SHADER_MACRO pm[] = { "FIRST_SHADER", "0", NULL, NULL };

		Engine::shader* triangleShader = Engine::ShaderManager::CompileAndCreateShader("Curlesque", L"Shaders/VertexShader.hlsl", L"Shaders/PixelShader.hlsl", nullptr, pm);
		if (!triangleShader)
			throw std::runtime_error("Failed to compile and create shader!");


		d3d->GetContext()->VSSetShader(triangleShader->vertexShader.Get(), nullptr, 0u);

		d3d->GetDevice()->CreateInputLayout(ied, 2u, triangleShader->vertexShaderBlob->GetBufferPointer(), triangleShader->vertexShaderBlob->GetBufferSize(), &pLayout);
		d3d->GetContext()->IASetInputLayout(pLayout.Get());

		d3d->GetContext()->PSSetShader(triangleShader->pixelShader.Get(), nullptr, 0u);


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

		d3d->GetDevice()->CreateBuffer(&bd, &sr_data, &pBuffer);

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
		HRESULT hr = d3d->GetDevice()->CreateBuffer(&cbd, &csd, &PSConstBuffer.m_constBuffer);
		assert(SUCCEEDED(hr));

		d3d->GetContext()->PSSetConstantBuffers(0u, 1u, PSConstBuffer.m_constBuffer.GetAddressOf());


		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		d3d->GetContext()->IASetVertexBuffers(0u, 1u,pBuffer.GetAddressOf(), &stride, &offset);
		d3d->GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	}
}

void D3DApplication::Update(float deltaTime)
{
	if (pWindow->wasWindowResized())
	{
		FLOAT windowWidth = (FLOAT)pWindow->getWindowWidth(), windowHeight = (FLOAT)pWindow->getWindowHeight();
		PSConstBuffer.constBufferData.g_resolution[0] = windowWidth;
		PSConstBuffer.constBufferData.g_resolution[1] = windowHeight;
		PSConstBuffer.constBufferData.g_resolution[2] = PSConstBuffer.constBufferData.g_resolution[3] = 1 / (windowWidth * windowHeight);
	}

	PSConstBuffer.constBufferData.g_time += deltaTime;

	PSConstBuffer.updateBuffer();

	const float color[] = { 0.5f, 0.5f,0.5f,1.0f };
	Engine::D3D* d3d = Engine::D3D::GetInstance();
	d3d->GetContext()->ClearRenderTargetView(pWindow->pRenderTarget.Get(), color);

	D3D11_BUFFER_DESC bd;
	pBuffer->GetDesc(&bd);
	d3d->GetContext()->Draw(bd.ByteWidth / (UINT)sizeof(Vertex), 0u);
	pWindow->flush();
}
