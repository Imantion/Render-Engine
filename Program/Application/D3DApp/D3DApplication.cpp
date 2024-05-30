#include "D3DApplication.h"
#include "Graphics/Engine.h"
#include "Graphics/ShaderManager.h"
#include "Input/Input.h"
#include "Render/Camera.h"
#include "Math/hitable.h"
#include "Math/math.h"
#include "Graphics/Renderer.h"
#include "Math/quaternion.h"
#include "Graphics/MeshSystem.h"
#include <assert.h>

Engine::vec2 previousMousePosition;
static float cameraSpeed = 2.0f;

D3DApplication::D3DApplication(int windowWidth, int windowHeight, WinProc windowEvent)
{
	camera.reset(new Engine::Camera(45.0f, 0.1f, 100.0f));
	pWindow.reset(new Engine::Window(windowWidth, windowHeight, windowEvent));
	camera->calculateProjectionMatrix(windowWidth, windowHeight);

	Engine::MeshSystem::Material knightMat = { Engine::vec3(1.0,0.0f,1.0f), 0.0f,Engine::vec3(1.0,1.0f,0.0f), 0.0f };

	auto changepos = [](Engine::MeshSystem::Instance& inst, const Engine::vec3& pos) {
		for (size_t i = 0; i < 3; i++)
		{
			inst.tranformation[3][i] = pos[i];
		}
		};

	auto model = Engine::ModelManager::GetInstance()->loadModel("Models\\Samurai.fbx");
	Engine::MeshSystem::Instance inst = { Engine::transformMatrix(Engine::vec3(0.0f, -1.0f, 0.0f), Engine::vec3(0.0f, 0.0f, 1.0f), Engine::vec3(1.0f, 0.0f, 0.0f), Engine::vec3(0.0f, 1.0f, 0.0f)) };
	Engine::MeshSystem::Init()->hologramGroup.addModel(model, knightMat, inst);

	knightMat = { Engine::vec3(0.0,1.0f,1.0f), 0.0f,Engine::vec3(1.0,0.0f,0.0f), 0.0f };
	changepos(inst, Engine::vec3(1.0f, -1.0f, 0.0f));
	Engine::MeshSystem::Init()->hologramGroup.addModel(model, knightMat, inst);

	model = Engine::ModelManager::GetInstance()->loadModel("Models\\cube.obj");
	changepos(inst, Engine::vec3(1.0f, 1.0f, 5.0f));
	Engine::MeshSystem::Init()->normVisGroup.addModel(model, knightMat, inst);

	changepos(inst, Engine::vec3(3.0f, -1.0f, -2.0f));
	Engine::MeshSystem::Init()->normVisGroup.addModel(model, knightMat, inst);

	changepos(inst, Engine::vec3(2.0f, -2.0f, 4.0f));
	Engine::MeshSystem::Init()->normVisGroup.addModel(model, knightMat, inst);

	changepos(inst, Engine::vec3(-4.0f, 0.0f, 1.0f));
	Engine::MeshSystem::Init()->normVisGroup.addModel(model, knightMat, inst);

	Engine::MeshSystem::Init()->normVisGroup.updateInstanceBuffers();
	Engine::MeshSystem::Init()->hologramGroup.updateInstanceBuffers();
}

bool D3DApplication::isClosed()
{
	return pWindow->isClosed();
}

void D3DApplication::Update(float deltaTime)
{
	UpdateInput(deltaTime);
	if (pWindow->wasWindowResized())
	{
		camera->calculateProjectionMatrix(pWindow->getWindowWidth(), pWindow->getWindowHeight());

	}

	Engine::D3D* d3d = Engine::D3D::GetInstance();
	Engine::Renderer* renderer = Engine::Renderer::GetInstance();
	renderer->updatePerFrameCB(deltaTime, (FLOAT)pWindow->getWindowWidth(), (FLOAT)pWindow->getWindowHeight());
	renderer->Render(camera.get());

	pWindow->flush();
}

void D3DApplication::UpdateInput(float deltaTime)
{
	Engine::vec2 mousePosition = Input::getMousePosition();

	Engine::vec3 cameraMoveDirection = (0.0f, 0.0f, 0.0f);
	if (Input::keyIsDown(Input::KeyboardButtons::W))
		cameraMoveDirection += camera->getForward() * deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::A))
		cameraMoveDirection += camera->getRight() * -deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::S))
		cameraMoveDirection += camera->getForward() * -deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::D))
		cameraMoveDirection += camera->getRight() * deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::Q))
		cameraMoveDirection += camera->getUp() * -deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::E))
		cameraMoveDirection += camera->getUp() * deltaTime;
	if (Input::keyIsDown(Input::KeyboardButtons::SHIFT))
		cameraMoveDirection *= 5;
	if (Input::mouseWasPressed(Input::MouseButtons::LEFT))
		previousMousePosition = mousePosition;



	int scrolls = Input::scrollAmount();
	if (scrolls > 0)
		cameraSpeed += cameraSpeed * 0.1f * scrolls;
	else if (scrolls < 0)
		cameraSpeed += cameraSpeed * 0.1f * scrolls;




	camera->moveCamera(cameraMoveDirection * cameraSpeed);

	bool cameraRotated = false;
	Engine::vec2 delta;
	if (Input::mouseIsDown(Input::MouseButtons::LEFT))
	{
		delta = (mousePosition - previousMousePosition) * (0.01f * deltaTime);
		if (delta.x != 0 || delta.y != 0)
		{
			Engine::quaternion q = (Engine::quaternion::angleAxis(delta.y, camera->getRight()) *
			Engine::quaternion::angleAxis(delta.x, camera->getUp())).normalize();
			camera->setForward(Engine::quaternion::rotate(q, camera->getForward()));
			cameraRotated = true;
		}

	}

	if (Input::mouseWasPressed(Input::MouseButtons::RIGHT))
	{
		Engine::vec2 screenCoord(mousePosition.x, pWindow->getWindowHeight() - mousePosition.y);
		Engine::ray r;
		screenCoord.x = (screenCoord.x / pWindow->getWindowWidth() - 0.5f) * 2.0f;
		screenCoord.y = (screenCoord.y / pWindow->getWindowHeight() - 0.5f) * 2.0f;
		r.origin = camera->getPosition();
		r.direction = camera->calculateRayDirection(screenCoord);

		Engine::hitInfo hInfo; hInfo.reset_parameter_t();
		auto instances = Engine::MeshSystem::Init()->intersect(r, hInfo);

		if(instances.size() > 0)
			dragger = std::make_unique<Engine::IInstanceDragger>(std::move(instances), hInfo);
				
	}
	else if (!Input::mouseIsDown(Input::MouseButtons::RIGHT))
	{
		dragger.release();
	}

	if (dragger)
	{
		Engine::vec2 screenCoord(mousePosition.x, pWindow->getWindowHeight() - mousePosition.y);
		Engine::ray r;
		screenCoord.x = (screenCoord.x / pWindow->getWindowWidth() - 0.5f) * 2.0f;
		screenCoord.y = (screenCoord.y / pWindow->getWindowHeight() - 0.5f) * 2.0f;
		r.origin = camera->getPosition();
		r.direction = camera->calculateRayDirection(screenCoord);

		if (delta.x != 0 || delta.y != 0)
		{
			Engine::quaternion q = (Engine::quaternion::angleAxis(delta.y, camera->getRight()) *
									Engine::quaternion::angleAxis(delta.x, camera->getUp())).normalize();
			r.direction = Engine::quaternion::rotate(q, r.direction);
		}
		dragger->drag(r);
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
