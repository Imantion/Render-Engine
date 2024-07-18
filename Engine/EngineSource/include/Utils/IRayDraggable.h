#pragma once
#include "Math/math.h"
#include "Graphics/MeshSystem.h"

namespace Engine
{

	class IRayDraggable
	{
	public:

		virtual void drag(const ray& r) = 0;
		uint32_t getObjectID() { return hittedObjectID; }

	protected:
		uint32_t hittedObjectID;
	};

	class IInstanceDragger : public IRayDraggable
	{
	public:
		IInstanceDragger(uint32_t hitId, const hitInfo& hInfo);

		virtual void drag(const ray& r);
	

	private:
		hitInfo grabbedInfo;
		std::vector<vec3> grabbedVectors;
	};
}

