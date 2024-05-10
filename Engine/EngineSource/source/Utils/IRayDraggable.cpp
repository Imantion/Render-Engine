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

Engine::IInstanceDragger::IInstanceDragger(std::vector<mat4*>&& transformationInstances, const hitInfo& hInfo)
{
	instances = std::move(transformationInstances);
	grabbedVectors.reserve(instances.size());
	grabbedInfo = hInfo;

	for (size_t i = 0; i < instances.size(); i++)
	{
		grabbedVectors.push_back(grabbedInfo.p - vec3((*instances[i])[3][0], (*instances[i])[3][1], (*instances[i])[3][2])); // from transform matrix bottom coordinates defines position
	}

}

void Engine::IInstanceDragger::drag(const ray& r)
{
	for (size_t i = 0; i < instances.size(); i++)
	{
		vec3 newPosition = r.point_at_parameter(grabbedInfo.t) - grabbedVectors[i];

		(*instances[i])[3][0] = newPosition.x;
		(*instances[i])[3][1] = newPosition.y;
		(*instances[i])[3][2] = newPosition.z;
	}

	MeshSystem::Init()->updateInstanceBuffers();
}