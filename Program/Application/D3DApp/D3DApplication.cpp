#include "D3DApplication.h"
#include "Graphics/Engine.h"
#include "Graphics/ShaderManager.h"
#include "Input/Input.h"
#include "Render/Camera.h"
#include <assert.h>
#include "Math/math.h"
#include "Graphics/Renderer.h"
#include "Math/quaternion.h"

Engine::vec2 previousMousePosition;

D3DApplication::D3DApplication(int windowWidth, int windowHeight, WinProc windowEvent)
{
	camera.reset(new Engine::Camera(90.0f, 0.1f, 100.0f));
	pWindow.reset(new Engine::Window(windowWidth, windowHeight, windowEvent));
}

void D3DApplication::PrepareTriangle()
{
	if (Engine::D3D* d3d = Engine::D3D::GetInstance())
	{
		D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};


		Engine::shader* triangleShader = Engine::ShaderManager::CompileAndCreateShader("Triangle", L"Shaders/VertexShader.hlsl", L"Shaders/PixelShader.hlsl", ied, 2u, nullptr, psMacro);
		if (!triangleShader)
			throw std::runtime_error("Failed to compile and create shader!");


		triangleShader->BindShader();

		Vertex OutVertices[] =
		{
			{0.0f, 0.5f, 0.9f, {1.0f, 0.0f, 0.0f, 1.0f}},
			{0.45f, -0.5, 0.9f,{0.0f, 1.0f, 0.0f, 1.0f}},
			{-0.45f, -0.5f,0.9f,{0.0f, 0.0f, 1.0f, 1.0f}},
		};

		vertexBuffer.create(OutVertices, 3);
		
		ConstantBuffer cb = {{800.0f,400.0f, 0.0f, 0.0f}, 0.0f };


		auto d =  dx::XMMatrixPerspectiveFovLH(3.14f / 2.0f,2.0f,10.0f,0.05f);

		/*Projection pj = { Engine::projectionMatrix(3.14f / 2.0f, 0.1f, 10.0f, 800, 400) };

		PSConstBuffer.create(&cb);
		VSConstBuffer.create(&pj);*/


		unsigned int indecies[] = { 0,1,2 };
		indexBuffer.create(indecies, 3);

		indexBuffer.bind();

		d3d->GetContext()->PSSetConstantBuffers(0u, 1u, PSConstBuffer.m_constBuffer.GetAddressOf());
		d3d->GetContext()->VSSetConstantBuffers(0u, 1u, VSConstBuffer.m_constBuffer.GetAddressOf());

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		vertexBuffer.bind(0u);
		d3d->GetContext()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}

void D3DApplication::PrepareSecondTriangle()
{
	if (Engine::D3D* d3d = Engine::D3D::GetInstance())
	{
		D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		D3D_SHADER_MACRO psMacro[] = { "FIRST_SHADER", "1", NULL, NULL };

		Engine::shader* triangleShader = Engine::ShaderManager::CompileAndCreateShader("Triangle", L"Shaders/VertexShader.hlsl", L"Shaders/PixelShader.hlsl", ied, 2u, nullptr, psMacro);
		if (!triangleShader)
			throw std::runtime_error("Failed to compile and create shader!");


		triangleShader->BindShader();

		Vertex OutVertices[] =
		{
		{ -1.0f,-1.0f,5.0f	,{1.0f,0.0f,0.0f,1.0f}},
		{ 1.0f,-1.0f,5.0f	 ,{1.0f,1.0f,0.0f,1.0f}},
		{ -1.0f,1.0f,5.0f	 ,{1.0f,1.0f,1.0f,1.0f}},
		{ 1.0f,1.0f,5.0f	 ,{1.0f,0.0f,1.0f,1.0f} },
		{ -1.0f,-1.0f,7.0f	 ,{0.0f,1.0f,0.0f,1.0f}},
		{ 1.0f,-1.0f,7.0f	 ,{0.0f,1.0f,1.0f,1.0f} },
		{ -1.0f,1.0f,7.0f	,{0.0f,0.0f,1.0f,1.0f} },
		{ 1.0f,1.0f,7.0f	,{0.0f,0.0f,0.0f,1.0f} },
		};

		vertexBuffer.create(OutVertices, 8);
		ConstantBuffer cb = { {800.0f,400.0f, 0.0f, 0.0f}, 0.0f };
		
		Engine::mat4 translation = {
			Engine::vec4(1,0,0,0),
			Engine::vec4(0,1,0,0),
			Engine::vec4(0,0,1,0),
			Engine::vec4(4,-3,0,1)
		};
		auto a = camera->getPosition();
		auto first = dx::XMVectorSet(a.x, a.y, a.z, 1.0f);
		auto second = dx::XMVectorSet(a.x, a.y, a.z + 1.0f, 1.0f);
		auto third = dx::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		auto testMat = camera->getViewMatrix();
		auto testDx = dx::XMMatrixLookAtLH(first, second, third);
		Projection pj = { translation * testMat * Engine::projectionMatrix(3.14f / 2.0f, 0.1f, 10.0f, 800, 400)};
		/*Projection pj = {Engine::projectionMatrix(3.14f / 2.0f, 0.1f, 10.0f, 800, 400) };*/

		PSConstBuffer.create();
		VSConstBuffer.create();

		unsigned int indecies[] = {
		0,2,1, 2,3,1,
		1,3,5, 3,7,5,
		2,6,3, 3,6,7,
		4,5,7, 4,7,6,
		0,4,2, 2,4,6,
		0,1,4, 1,5,4
		};
		indexBuffer.create(indecies, 36);

		indexBuffer.bind();

		d3d->GetContext()->PSSetConstantBuffers(0u, 1u, PSConstBuffer.m_constBuffer.GetAddressOf());
		d3d->GetContext()->VSSetConstantBuffers(0u, 1u, VSConstBuffer.m_constBuffer.GetAddressOf());

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
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		D3D_SHADER_MACRO pm[] = { "FIRST_SHADER", "0", NULL, NULL };

		Engine::shader* triangleShader = Engine::ShaderManager::CompileAndCreateShader("Curlesque", L"Shaders/VertexShader.hlsl", L"Shaders/PixelShader.hlsl",ied, 2u, nullptr, pm);
		if (!triangleShader)
			throw std::runtime_error("Failed to compile and create shader!");

		triangleShader->BindShader();

		Vertex OutVertices[] =
		{
			{-1.0f, -1.0f,1.0f,  {1.0f, 0.0f, 0.0f, 1.0f}},
			{-1.0f, 1.0f,1.0f, {0.0f, 1.0f, 0.0f, 1.0f}},
			{1.0f, -1.0f,1.0f,{0.0f, 0.0f, 1.0f, 1.0f}},
			{1.0f, 1.0f,1.0f,{0.0f, 0.0f, 1.0f, 1.0f}},
		};

		vertexBuffer.create(OutVertices, 4);

		PSConstBuffer.create();

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
	UpdateInput(deltaTime);
	ConstantBuffer constBufferData;
	FLOAT windowWidth = (FLOAT)pWindow->getWindowWidth(), windowHeight = (FLOAT)pWindow->getWindowHeight();
	constBufferData.g_resolution[0] = windowWidth;
	constBufferData.g_resolution[1] = windowHeight;
	constBufferData.g_resolution[2] = constBufferData.g_resolution[3] = 1 / (windowWidth * windowHeight);
	constBufferData.g_time += deltaTime;

	PSConstBuffer.updateBuffer(&constBufferData);
	Engine::mat4 translation = {
			Engine::vec4(1,0,0,0),
			Engine::vec4(0,1,0,0),
			Engine::vec4(0,0,1,0),
			Engine::vec4(4,-3,0,1)
	};

	auto a = camera->getPosition();
	auto b = camera->getForward();
	auto first = dx::XMVectorSet(a.x, a.y, a.z, 1.0f);
	auto second = dx::XMVectorSet(a.x + b.x, a.y + b.y, a.z + b.z, 1.0f);
	auto third = dx::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	auto testMat = camera->getViewMatrix();
	auto testDx = dx::XMMatrixLookAtLH(first, second, third);

	Projection pj = { translation * camera->getViewMatrix() * Engine::projectionMatrix(3.14f / 3.0f, 100.0f, 0.1f, 800, 400) };
	VSConstBuffer.updateBuffer(&pj);

	Engine::D3D* d3d = Engine::D3D::GetInstance();
	Engine::Renderer::GetInstance()->Render();
	d3d->GetContext()->DrawIndexed(indexBuffer.getSize(), 0u, 0u);


	pWindow->flush();
}

void D3DApplication::UpdateInput(float deltaTime)
{
	Engine::vec2 mousePosition = Input::getMousePosition();
	Engine::vec2 delta = (mousePosition - previousMousePosition) * 0.005f;
	previousMousePosition = Input::getMousePosition();

	float cameraSpeed = 5.0f;

	Engine::vec3 cameraMoveDirection = (0.0f, 0.0f, 0.0f);
	if (Input::keyIsDown(Input::KeyboardButtons::W))
		cameraMoveDirection += camera->getForward() * cameraSpeed * deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::A))
		cameraMoveDirection += camera->getRight() * cameraSpeed * -deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::S))
		cameraMoveDirection += camera->getForward() * cameraSpeed * -deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::D))
		cameraMoveDirection += camera->getRight() * cameraSpeed * deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::CTRL))
		cameraMoveDirection += camera->getUp() * cameraSpeed * -deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::SPACE))
		cameraMoveDirection += camera->getUp() * cameraSpeed * deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::SHIFT))
		cameraMoveDirection *= 5;
	//if (Input::keyIsDown(Input::KeyboardButtons::E))
	//	roll -= 0.5f * deltaTime;
	//if (Input::keyIsDown(Input::KeyboardButtons::Q))
	//	roll += 0.5f * deltaTime;



	int scrolls = Input::scrollAmount();
	if (scrolls > 0)
		cameraSpeed += cameraSpeed * 0.05f * scrolls;
	else if (scrolls < 0)
		cameraSpeed += cameraSpeed * 0.05f * scrolls;


	

	camera->moveCamera(cameraMoveDirection);

	bool cameraRotated = false;

	if (Input::mouseIsDown(Input::MouseButtons::LEFT))
	{

		if (delta.x != 0 || delta.y != 0)
		{
			Engine::quaternion q = (Engine::quaternion::angleAxis(delta.y, camera->getRight()) *
								    Engine::quaternion::angleAxis(delta.x, camera->getUp())).normalize();
			camera->setForward(Engine::quaternion::rotate(q, camera->getForward()));
			cameraRotated = true;
		}
	}

	if (cameraMoveDirection != Engine::vec3(0.0f) || cameraRotated)
	{
		camera->calculateViewMatrix();
		camera->setRight(Engine::cross(camera->getUp(), camera->getForward()));
	}
}

D3DApplication::~D3DApplication()
{
}
