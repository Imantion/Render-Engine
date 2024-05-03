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

		Engine::shader* triangleShader = Engine::ShaderManager::CompileAndCreateShader("Triangle", L"Shaders/VertexShader.hlsl", L"Shaders/PixelShader.hlsl", ied, nullptr, psMacro);
		if (!triangleShader)
			throw std::runtime_error("Failed to compile and create shader!");


		triangleShader->BindShader();

		Vertex OutVertices[] =
		{
			{0.0f, 0.5f,  {1.0f, 0.0f, 0.0f, 1.0f}},
			{0.45f, -0.5, {0.0f, 1.0f, 0.0f, 1.0f}},
			{-0.45f, -0.5f,{0.0f, 0.0f, 1.0f, 1.0f}}
		};

		vertexBuffer.create(OutVertices, 3);
		
		ConstantBuffer cb = {{800.0f,400.0f, 0.0f, 0.0f}, 0.0f };

		PSConstBuffer.create(&cb);

		d3d->GetContext()->PSSetConstantBuffers(0u, 1u, PSConstBuffer.m_constBuffer.GetAddressOf());

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		vertexBuffer.bind(0u);
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

		Engine::shader* triangleShader = Engine::ShaderManager::CompileAndCreateShader("Curlesque", L"Shaders/VertexShader.hlsl", L"Shaders/PixelShader.hlsl",ied, nullptr, pm);
		if (!triangleShader)
			throw std::runtime_error("Failed to compile and create shader!");

		triangleShader->BindShader();

		Vertex OutVertices[] =
		{
			{-1.0f, -1.0f,  {1.0f, 0.0f, 0.0f, 1.0f}},
			{-1.0f, 1.0f, {0.0f, 1.0f, 0.0f, 1.0f}},
			{1.0f, -1.0f,{0.0f, 0.0f, 1.0f, 1.0f}},
			{1.0f, 1.0f,{0.0f, 0.0f, 1.0f, 1.0f}},
		};

		vertexBuffer.create(OutVertices, 4);

		ConstantBuffer cb = {{800.0f,400.0f, 0.0f, 0.0f}, 0.0f };

		PSConstBuffer.create(&cb);

		d3d->GetContext()->PSSetConstantBuffers(0u, 1u, PSConstBuffer.m_constBuffer.GetAddressOf());


		UINT stride = sizeof(Vertex);
		UINT offset = 0u;

		vertexBuffer.bind(0u);
		d3d->GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	}
}

bool D3DApplication::isClosed()
{
	return pWindow->isClosed();
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

	d3d->GetContext()->Draw(vertexBuffer.getSize(), 0u);
	pWindow->flush();
}

D3DApplication::~D3DApplication()
{
}
