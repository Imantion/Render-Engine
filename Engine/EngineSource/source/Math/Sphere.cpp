#include "Math/Sphere.h"
#include "Math/math.h"
#include "math.h"
using namespace Engine;

bool Engine::Sphere::hit(const ray& r, float t_min, float t_max, hitInfo& hInfo) const
{
	vec3 oc = r.origin - position;
	float a = dot(r.direction, r.direction);
	float b = 2 * dot(oc, r.direction);
	float c = dot(oc, oc) - radius * radius;
	float d = b * b - 4 * a * c;

	if (d >= 0)	
	{
		float root_d = sqrtf(d);
		float t = (-b - root_d) / (2 * a);
		if (t < t_max && t > t_min)
		{
			hInfo.t = t;
			hInfo.p = r.point_at_parameter(t);
			hInfo.normal = (hInfo.p - position) / radius;
			return true;
		}

		t = (-b - root_d) / (2 * a);

		if (t < t_max && t > t_min)
		{
			hInfo.t = t;
			hInfo.p = r.point_at_parameter(t);
			hInfo.normal = (hInfo.p - position) / radius;
			return true;
		}
	}

	return false;
}
