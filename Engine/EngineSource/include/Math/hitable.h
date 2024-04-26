#pragma once
#include <limits>
#include <cmath>

namespace Engine
{
	struct hitInfo
	{
		float t;
		vec3 p;
		vec3 normal;

		
		void reset_parameter_t() { this->t = (std::numeric_limits<float>::infinity)(); }
		bool is_t_finite() { return std::isfinite(t); }
	};

	enum class IntersectedType { undefined, sphere, primitive, plane, pointLight, spotLight };

	struct objectRef
	{
		void* pObject;
		IntersectedType pObjectType;

		void reset()
		{
			pObject = nullptr;
			pObjectType = IntersectedType::undefined;
		}
	};

}