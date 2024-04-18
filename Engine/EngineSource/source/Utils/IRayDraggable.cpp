#include "Utils/IRayDraggable.h"
#include "Render/Scene.h"

Engine::ISphereDragger::ISphereDragger(sphere* s, const hitInfo& hInfo)
{
	grabbedSphere = s;
	grabbedInfo = hInfo;
	grabbedVector = s->position - hInfo.p;
}

void Engine::ISphereDragger::drag(const ray& r, const vec3& cameraForward, const vec3& cameraMoveDirection)
{
	Engine::hitInfo planeInfo;

	if (Engine::hitPlane(-cameraForward, r, planeInfo, grabbedInfo.p))
	{
		grabbedInfo.p = planeInfo.p + cameraMoveDirection;
		grabbedSphere->position = grabbedInfo.p + grabbedVector;
	}
}

Engine::IMeshDragger::IMeshDragger(primitive* primitiveToDrag, const hitInfo& hInfo)
{
	grabbedPrim = primitiveToDrag;
	grabbedInfo = hInfo;
	grabbedVector = primitiveToDrag->getPosition() - hInfo.p;
}

void Engine::IMeshDragger::drag(const ray& r, const vec3& cameraForward, const vec3& cameraMoveDirection)
{
	Engine::hitInfo planeInfo;

	if (Engine::hitPlane(-cameraForward, r, planeInfo, grabbedInfo.p))
	{
		grabbedInfo.p = planeInfo.p + cameraMoveDirection;
		grabbedPrim->setPosition(grabbedInfo.p + grabbedVector);
	}
}
