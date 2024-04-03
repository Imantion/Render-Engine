#pragma once
#include "Math/hitable.h"


namespace Engine
{
	class Sphere : public hitable
	{
	public:
		Sphere(vec3 pos, float r) : position(pos), radius(r) {}
		Sphere(const Sphere& s) : position(s.position), radius(s.radius) {}

		virtual bool hit(const ray& r, float t_min, float t_max, hitInfo& hInfo) const override;


		vec3 position;
		float radius;
	};

}