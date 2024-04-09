#pragma once
#include "Math/vec.h"
#include "Math/matrix.h"
#include "hitable.h"
#include <math.h>

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

	inline bool hitPlane(const vec3& normal, const ray& ray, const vec3& point = vec3(0,0,0))
	{
		float denom = dot(normal, ray.direction);
		if (denom > 1e-6)
		{
			float t = dot(point - ray.origin, normal) / denom;

			return t >= 0;
		}
		return false;
	}

	inline mat4 projectionMatrix(float verticalFov, float nearClip, float farClip, int viewportWidth, int viewportHeight)
	{
		mat4 projMat;
		float halfFov = verticalFov * 0.5;
		float ctg = cos(halfFov) / sin(halfFov);

		/*projMat[0][0] = (float)viewportHeight / (float)viewportWidth * ctg;
		projMat[1][1] = ctg;
		projMat[2][2] = nearClip / (nearClip - farClip);
		projMat[2][3] = 1;
		projMat[3][2] = -farClip * nearClip / (nearClip - farClip);*/


		projMat[0][0] = (float)viewportHeight / (float)viewportWidth * ctg;
		projMat[1][1] = ctg;
		projMat[2][2] = (farClip + nearClip) / (nearClip - farClip);
		projMat[2][3] = 1;
		projMat[3][2] = -2 * farClip * nearClip / (nearClip - farClip);

		return projMat;
	}

	inline mat4 transformMatrix(vec3 eye, vec3 center, vec3 upVector)
	{
		vec3 f((center - eye).normalized());
		vec3 rightVector(cross(upVector, f).normalized());
		vec3 u(cross(f, rightVector));

		mat4 matrix;

		matrix[0][0] = rightVector.x;
		matrix[0][1] = rightVector.y;
		matrix[0][2] = rightVector.z;

		matrix[1][0] = u.x;
		matrix[1][1] = u.y;
		matrix[1][2] = u.z;

		matrix[2][0] = f.x;
		matrix[2][1] = f.y;
		matrix[2][2] = f.z;

		matrix[3][0] = eye.x;
		matrix[3][1] = eye.y;
		matrix[3][2] = eye.z;
		matrix[3][3] = 1;

		return matrix;
	}

	inline mat4 inverseTransformMatrix(vec3 eye, vec3 center, vec3 upVector)
	{
		vec3 f((center - eye).normalized());
		vec3 rightVector(cross(upVector, f).normalized());
		vec3 u(cross(f, rightVector));
		mat4 matrix;

		matrix[0][0] = rightVector.x;
		matrix[0][1] = upVector.x;
		matrix[0][2] = f.x;

		matrix[1][0] = rightVector.y;
		matrix[1][1] = upVector.y;
		matrix[1][2] = f.y;

		matrix[2][0] = rightVector.z;
		matrix[2][1] = upVector.z;
		matrix[2][2] = f.z;

		matrix[3][0] = -dot(rightVector, eye);
		matrix[3][1] = -dot(upVector, eye);
		matrix[3][2] = -dot(f, eye);
		matrix[3][3] = 1;

		return matrix;
	}
}