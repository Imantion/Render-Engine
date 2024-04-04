#pragma once
#include "Math/vec.h"
#include "hitable.h"

namespace Engine
{
	struct sphere
	{
		sphere(vec3 pos, float r) : position(pos), radius(r) {}
		sphere(const sphere& s) : position(s.position), radius(s.radius) {}


		vec3 position;
		float radius;
	};

	struct ray
	{
		vec3 origin;
		vec3 direction;

		ray(vec3 pos, vec3 dir) : origin(pos), direction(dir) {}

		vec3 point_at_parameter(float t) const
		{
			return origin + (direction * t);
		}
	};

	inline float dot(const vec3& a, const vec3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	inline vec3 cross(const vec3& a, const vec3& v)
	{
		return vec3(a.y * v.z - a.z * v.y, a.z * v.x - a.x * v.z, a.x * v.y - a.y * v.x);
	}

	inline bool hitSphere(const sphere& s, const ray& r, float t_min, float t_max, hitInfo& hInfo)
	{
		vec3 oc = r.origin - s.position;
		float a = dot(r.direction, r.direction);
		float b = 2 * dot(oc, r.direction);
		float c = dot(oc, oc) - s.radius * s.radius;
		float d = b * b - 4 * a * c;

		if (d >= 0)
		{
			float root_d = sqrtf(d);
			float t = (-b - root_d) / (2 * a);
			if (t < t_max && t > t_min)
			{
				hInfo.t = t;
				hInfo.p = r.point_at_parameter(t);
				hInfo.normal = (hInfo.p - s.position) / s.radius;
				return true;
			}

			t = (-b - root_d) / (2 * a);

			if (t < t_max && t > t_min)
			{
				hInfo.t = t;
				hInfo.p = r.point_at_parameter(t);
				hInfo.normal = (hInfo.p - s.position) / s.radius;
				return true;
			}
		}

		return false;
	}

}