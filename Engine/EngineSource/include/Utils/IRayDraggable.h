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

		virtual void drag(const ray& r, const vec3& cameraForward, const vec3& cameraMoveDirection) = 0;
	};

	class ISphereDragger : public IRayDraggable
	{
	public:

		ISphereDragger(sphere* s, const hitInfo& hInfo);
		virtual void drag(const ray& r, const vec3& cameraForward, const vec3& cameraMoveDirection = vec3(0.0f)) override;
	private:
		sphere* grabbedSphere;
		hitInfo grabbedInfo;
		vec3 grabbedVector;
	};

	class IMeshDragger : public IRayDraggable
	{
	public:
		IMeshDragger(primitive* primitiveToDrag, const hitInfo& hInfo);
		virtual void drag(const ray& r, const vec3& cameraForward, const vec3& cameraMoveDirection = vec3(0.0f)) override;
	private:
		primitive* grabbedPrim;
		hitInfo grabbedInfo;
		vec3 grabbedVector;
	};
}

