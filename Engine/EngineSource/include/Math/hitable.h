#pragma once
#include <limits>

namespace Engine
{
	struct hitInfo
	{
		float t;
		vec3 p;
		vec3 normal;

		
		void reset_parameter_t() { this->t = (std::numeric_limits<float>::max)(); }
	};

}