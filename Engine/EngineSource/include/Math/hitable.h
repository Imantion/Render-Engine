#pragma once
#include <limits>

namespace Engine
{
	struct hitInfo
	{
		float t;
		vec3 p;
		vec3 normal;

		

		hitInfo& operator=(const hitInfo& other) {
			t = other.t;
			p = other.p;
			normal = other.normal;

			return *this;
		}
		#undef max
		void reset_parameter_t() { this->t = std::numeric_limits<float>::max(); }
	};

}