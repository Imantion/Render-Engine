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

