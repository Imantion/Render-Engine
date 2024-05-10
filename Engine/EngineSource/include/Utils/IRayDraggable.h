#pragma once
#include "Math/math.h"

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
		IInstanceDragger(I* instance) {}
	};
}

