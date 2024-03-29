#pragma once
#include "vec.h"

namespace Engine
{
	struct sphere
	{
		vec3 position;
		float radius;

		sphere(vec3 pos, float r) : position(pos), radius(r) {}
	};

	struct ray
	{
		vec3 position;
		vec3 direction;

		ray(vec3 pos, vec3 dir) : position(pos), direction(dir) {}
	};

	float dot(const vec3& a, const vec3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	vec3 cross(const vec3& a, const vec3& v)
	{
		return vec3(a.y * v.z - a.z * v.y, a.z * v.x - a.x * v.z, a.x * v.y - a.y * v.x);
	}
}
