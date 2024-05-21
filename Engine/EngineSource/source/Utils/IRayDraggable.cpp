#include "Utils/IRayDraggable.h"

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
		vec3 translatedPosition = vec4(r.point_at_parameter(grabbedInfo.t), 0.0f);
		vec3 newPosition = (translatedPosition - grabbedVectors[i]);

		(*instances[i])[3][0] = newPosition.x;
		(*instances[i])[3][1] = newPosition.y;
		(*instances[i])[3][2] = newPosition.z;
	}

	MeshSystem::Init()->updateInstanceBuffers();
}