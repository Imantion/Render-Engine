#pragma once
#include <memory>
#include "Window/Window.h"
#include "Graphics/D3D.h"
#include "Graphics/Buffers.h"


struct Vertex
{
	FLOAT x, y;
	FLOAT color[4];
};

struct ConstantBuffer
{
	float g_resolution[4];
	float g_time;

	float padding[3];
};

class D3DApplication
{
public:
	D3DApplication(int windowSize, int windowHeight, WinProc);

	void PrepareTriangle();
	void PrepareCurlesque();
	bool isClosed();

	void Update(float deltaTime);

	
	~D3DApplication();

private:
	std::unique_ptr<Engine::Window> pWindow;
	Engine::VertexBuffer<Vertex> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pLayout;
	Engine::ConstBuffer<ConstantBuffer> PSConstBuffer;
};

