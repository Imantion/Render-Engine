#include "Utils/IRayDraggable.h"
#include "Graphics/TransformSystem.h"

Engine::IInstanceDragger::IInstanceDragger(uint32_t hitId, const hitInfo& hInfo)
{
	hittedObjectID = hitId;
	auto TS = TransformSystem::Init();
	auto& transforms = TS->GetModelTransforms(hittedObjectID);
	grabbedVectors.reserve(transforms.size());
	grabbedInfo = hInfo;

	for (size_t i = 0; i < transforms.size(); i++)
	{
		grabbedVectors.push_back(grabbedInfo.p - vec3(transforms[i].modelToWold[3][0], transforms[i].modelToWold[3][1], transforms[i].modelToWold[3][2])); // from transform matrix bottom coordinates defines position
	}

}

void Engine::IInstanceDragger::drag(const ray& r)
{
	auto TS = TransformSystem::Init();
	for (size_t i = 0; i < grabbedVectors.size(); i++)
	{
		vec3 translatedPosition = r.point_at_parameter(grabbedInfo.t);
		vec3 newPosition = (translatedPosition - grabbedVectors[i]);
		TS->SetModelMeshPosition(hittedObjectID, (uint32_t)i, newPosition);
	}

	MeshSystem::Init()->updateInstanceBuffers();
}