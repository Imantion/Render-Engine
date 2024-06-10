#pragma once
#include <memory>
#include <Math/matrix.h>
#include "Window/Window.h"
#include "Graphics/D3D.h"
#include "Graphics/Buffers.h"
#include "Utils/IRayDraggable.h"
#include <DirectXMath.h>

namespace dx = DirectX;
struct Vertex
{
	FLOAT x, y, z;
	FLOAT color[4];
};

struct ConstantBuffer
{
	float g_resolution[4];
	float g_time;

	float padding[3];
};

struct Projection
{
	Engine::mat4 matrix;
};

namespace Engine
{
	class Camera;
}

class D3DApplication
{
public:
	D3DApplication(int windowSize, int windowHeight, WinProc);

	bool isClosed();

	void Update(float deltaTime);
	void UpdateInput(float deltaTime);

	
	~D3DApplication();

private:
	std::shared_ptr<Engine::Camera> camera;
	std::unique_ptr<Engine::Window> pWindow;
	
	std::unique_ptr<Engine::IRayDraggable> dragger;
};

