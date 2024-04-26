#include "Utils/IRayDraggable.h"
#include "Render/Scene.h"

Engine::ISphereDragger::ISphereDragger(sphere* s, const hitInfo& hInfo)
{
	grabbedSphere = s;
	grabbedInfo = hInfo;
	grabbedVector = s->position - hInfo.p;
}

void Engine::ISphereDragger::drag(const ray& r)
{
	grabbedSphere->position = r.point_at_parameter(grabbedInfo.t) + grabbedVector;
}

Engine::IMeshDragger::IMeshDragger(primitive* primitiveToDrag, const hitInfo& hInfo)
{
	grabbedPrim = primitiveToDrag;
	grabbedInfo = hInfo;
	grabbedVector = primitiveToDrag->getPosition() - hInfo.p;
}

void Engine::IMeshDragger::drag(const ray& r)
{
	grabbedPrim->setPosition(r.point_at_parameter(grabbedInfo.t) + grabbedVector);
}
