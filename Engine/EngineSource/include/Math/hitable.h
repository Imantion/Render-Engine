#pragma once
#include "Math/math.h"

namespace Engine
{
	struct hitInfo
	{
		float t;
		vec3 p;
		vec3 normal;
	};

	class hitable
	{
	public:
		virtual bool hit(const ray& r, float t_min, float t_max, hitInfo& hInfo) const = 0;
	};
}