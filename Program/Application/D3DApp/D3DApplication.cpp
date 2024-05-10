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

D3DApplication::D3DApplication(int windowWidth, int windowHeight, WinProc windowEvent)
{
	camera.reset(new Engine::Camera(45.0f, 0.1f, 100.0f));
	pWindow.reset(new Engine::Window(windowWidth, windowHeight, windowEvent));
	camera->calculateProjectionMatrix(windowWidth, windowHeight);
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
	Engine::vec2 delta = (mousePosition - previousMousePosition) * 0.005f;
	previousMousePosition = Input::getMousePosition();

	float cameraSpeed = 2.0f;

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

	if (Input::mouseIsDown(Input::MouseButtons::RIGHT))
	{
		Engine::vec2 screenCoord(mousePosition.x, pWindow->getWindowHeight() - mousePosition.y);
		Engine::ray r;
		screenCoord.x = (screenCoord.x / pWindow->getWindowWidth() - 0.5f) * 2.0f;
		screenCoord.y = (screenCoord.y / pWindow->getWindowHeight() - 0.5f) * 2.0f;
		r.origin = camera->getPosition();
		r.direction = camera->calculateRayDirection(screenCoord);


		Engine::MeshSystem::Instance inst;
		Engine::hitInfo hInfo; hInfo.reset_parameter_t();

		Engine::MeshSystem::Init()->intersect(r, hInfo, inst);
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
