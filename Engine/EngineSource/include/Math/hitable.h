#pragma once
#include "Math/math.h"

namespace Engine
{
	struct hitInfo
	{
		float t;
		vec3 p;
		vec3 normal;

		hitInfo() : t(FLT_MAX) {}
		hitInfo& operator=(const hitInfo& other) {
			t = other.t;
			p = other.p;
			normal = other.normal;

			return *this;
		}
	};

}