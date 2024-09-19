#pragma once
#include "Math/vec.h"
#include <limits>
#undef max

namespace Engine
{
	struct Box
	{
		vec3 min;
		vec3 max;

		static constexpr float Inf = std::numeric_limits<float>::infinity();
		static  Box empty() { return  { { Inf, Inf, Inf }, { -Inf, -Inf, -Inf } }; }
		static  Box unit() { return  { { -1.f, -1.f, -1.f }, { 1.f, 1.f, 1.f } }; }

		float size() const { return (max - min).length(); }
		vec3 center() const { return (min + max) / 2.f; }
		float radius() const { return size() / 2.f; }

		void reset()
		{
			constexpr float maxf = std::numeric_limits<float>::max();
			min = { maxf , maxf , maxf };
			max = -min;
		}

		void expand(const Box& other)
		{
			min = min.minimum(other.min);
			max = max.maximum(other.max);
		}

		void expand(const vec3& point)
		{
			min = min.minimum(point);
			max = max.maximum(point);
		}

		bool contains(const vec3& P)
		{
			return
				min[0] <= P[0] && P[0] <= max[0] &&
				min[1] <= P[1] && P[1] <= max[1] &&
				min[2] <= P[2] && P[2] <= max[2];
		}
	};
}