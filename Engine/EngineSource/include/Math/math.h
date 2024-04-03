#pragma once

#include "vec.h"

namespace Engine
{
	struct sphere
	{
		vec3 position;
		float radius;

		sphere(vec3 pos, float r) : position(pos), radius(r) {}
		sphere(const sphere& s): position(s.position), radius(s.radius) {}
	};

	struct ray
	{
		vec3 origin;
		vec3 direction;

		ray(vec3 pos, vec3 dir) : origin(pos), direction(dir) {}
	};

	inline float dot(const vec3& a, const vec3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	inline vec3 cross(const vec3& a, const vec3& v)
	{
		return vec3(a.y * v.z - a.z * v.y, a.z * v.x - a.x * v.z, a.x * v.y - a.y * v.x);
	}

	inline float hitSphere(const ray& r, const sphere& s)
	{
		vec3 oc = r.origin - s.position;
		float a = dot(r.direction, r.direction);
		float b = 2 * dot(oc, r.direction);
		float c = dot(oc, oc) - s.radius * s.radius;

		float d = b * b - 4 * a * c;

		if (d < 0)
			return -1;

		return (-b - sqrtf(d)) / (2 * a);
	}
}