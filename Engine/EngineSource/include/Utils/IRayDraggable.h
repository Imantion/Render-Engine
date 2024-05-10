#pragma once
#include "Math/math.h"
#include "Graphics/MeshSystem.h"

namespace Engine
{
	class Scene;
	struct sphere;
	struct primitive;

	class IRayDraggable
	{
	public:

		virtual void drag(const ray& r) = 0;
	};

	class ISphereDragger : public IRayDraggable
	{
	public:

		ISphereDragger(sphere* s, const hitInfo& hInfo);
		virtual void drag(const ray& r) override;
	private:
		sphere* grabbedSphere;
		hitInfo grabbedInfo;
		vec3 grabbedVector;
	};

	class IMeshDragger : public IRayDraggable
	{
	public:
		IMeshDragger(primitive* primitiveToDrag, const hitInfo& hInfo);
		virtual void drag(const ray& r) override;
	private:
		primitive* grabbedPrim;
		hitInfo grabbedInfo;
		vec3 grabbedVector;
	};

	template<typename I>
	class IInstanceDragger : public IRayDraggable
	{
	public:
		IInstanceDragger(I* instance,const hitInfo& hInfo) {
			inst = instance;
			grabbedInfo = hInfo;

			vec3 instancePosition(inst->tranformation[3][0], inst->tranformation[3][1], inst->tranformation[3][2]); // from transform matrix bottom coordinates defines position
			grabbedVector = grabbedInfo.p - instancePosition;
		}

		virtual void drag(const ray& r)
		{
			vec3 newPosition = r.point_at_parameter(grabbedInfo.t) - grabbedVector;

			inst->tranformation[3][0] = newPosition.x;
			inst->tranformation[3][1] = newPosition.y;
			inst->tranformation[3][2] = newPosition.z;

			MeshSystem::Init()->hologramGroup.updateInstanceBuffers();
		}

	private:
		I* inst;
		hitInfo grabbedInfo;
		vec3 grabbedVector;
	};
}

