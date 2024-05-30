#pragma once
#include "Math/math.h"
#include "Graphics/MeshSystem.h"

namespace Engine
{

	class IRayDraggable
	{
	public:

		virtual void drag(const ray& r) = 0;
	};

	class IInstanceDragger : public IRayDraggable
	{
	public:
		IInstanceDragger(std::vector<mat4*>&& transformationInstances, const hitInfo& hInfo);

		virtual void drag(const ray& r);
	

	private:
		std::vector<mat4*> instances;
		hitInfo grabbedInfo;
		std::vector<vec3> grabbedVectors;
	};
}

