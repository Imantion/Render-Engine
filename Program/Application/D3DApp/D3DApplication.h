#pragma once
#include <memory>
#include <Math/matrix.h>
#include "Window/Window.h"
#include "Graphics/D3D.h"
#include "Graphics/Buffers.h"
#include "Utils/IRayDraggable.h"
#include "Graphics/SkyBox.h"
#include <DirectXMath.h>

namespace dx = DirectX;

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
	void InitCamera(int windowWidth, int windowHeight);
	void InitSamuraiModel();
	void InitLights();
	void InitCrateModel();
	void InitFloor();
	void InitSkybox();
	void InitPostProcess();

private:
	std::shared_ptr<Engine::Camera> camera;
	std::unique_ptr<Engine::Window> pWindow;
	
	std::unique_ptr<Engine::IRayDraggable> dragger;
	Engine::SkyBox skybox;
};

