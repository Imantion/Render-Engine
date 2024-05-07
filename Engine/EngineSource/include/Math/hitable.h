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
		float m_near;
		uint32_t triangle;

		constexpr void reset(float near_, float far_ = std::numeric_limits<float>::infinity())
		{
			this->m_near = near_;
			t = far_;
		}
		bool valid() const { return std::isfinite(t); }
		
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