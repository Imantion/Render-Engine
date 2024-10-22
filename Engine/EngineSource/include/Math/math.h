#pragma once
#define _USE_MATH_DEFINES

#include "Math/vec.h"
#include "Math/matrix.h"
#include "Math/hitable.h"
#include "Math/triangle.h"
#include <math.h>

namespace Engine
{
	struct mathSphere
	{
		mathSphere() : position(0.0f), radius(0.0f) {}
		mathSphere(vec3 pos, float r) : position(pos), radius(r) {}
		mathSphere(const mathSphere& s) : position(s.position), radius(s.radius) {}

		vec3 position;
		float radius;
	};

	struct mathPlane
	{
		mathPlane() {}
		mathPlane(const vec3& point, const vec3& normal) : point(point), normal(normal) {}

		vec3 point;
		vec3 normal;
	};


	struct ray
	{
		vec3 origin;
		vec3 direction;

		ray() : origin(0.0), direction(0.0f) {}
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

	inline bool hitSphere(const mathSphere& s, const ray& r, float t_min, float t_max, hitInfo& hInfo)
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
			if (t < t_max && t > t_min && t < hInfo.t)
			{
				hInfo.t = t;
				hInfo.p = r.point_at_parameter(t);
				hInfo.normal = (hInfo.p - s.position).normalized();
				return true;
			}

			t = (-b + root_d) / (2 * a);

			if (t < t_max && t > t_min && t < hInfo.t)
			{
				hInfo.t = t;
				hInfo.p = r.point_at_parameter(t);
				hInfo.normal = (hInfo.p - s.position).normalized();
				return true;
			}
		}

		return false;
	}

	inline bool hitPlane(const mathPlane& pln, const ray& ray, hitInfo& hInfo)
	{
		
		float denom = dot(-pln.normal, ray.direction);
		if (denom > 1e-6)
		{
			float t = dot(pln.point - ray.origin, -pln.normal) / denom;

			if (t > 0 && t < hInfo.t)
			{
				hInfo.normal = pln.normal;
				hInfo.p = ray.point_at_parameter(t);
				hInfo.t = t;
				return true;
			}
		}
		return false;
	}

	inline bool hitTriangle(const triangle& tr, const ray& r, hitInfo& hInfo)
	{
		vec3 E1 = tr.C - tr.A;
		vec3 E2 = tr.B - tr.A;
		vec3 P = cross(r.direction, E1);


		float determinant = dot(P, E2);

		if (determinant < 1e-6)
			return false;

		vec3 T = r.origin - tr.A;
		vec3 Q = cross(T, E2);

		float invDeterminant = 1.0f / determinant;

		float t = dot(Q, E1) * invDeterminant;

		if (t < 0 /*|| t > hInfo.t*/)
			return false;

		float u = dot(P, T) * invDeterminant;

		if (u > 1.0f || u < 0.0f)
			return false;

		float v = dot(Q, r.direction) * invDeterminant;

		if (v > 1.0f || v < 0.0f || u + v > 1.0f)
			return false;

		hInfo.p = r.point_at_parameter(t);
		hInfo.t = t;
		hInfo.normal = cross(E2, E1).normalized();

		return true;

	}


	inline vec3 clampVec(const vec3& v, float value)
	{
		vec3 clampedVector;

		clampedVector.x = v.x > value ? value : v.x;
		clampedVector.y = v.y > value ? value : v.y;
		clampedVector.z = v.z > value ? value : v.z;

		return clampedVector;
	}

	inline uint32_t ConvertToRGB(const vec3& color)
	{
		vec3 clampedColor = clampVec(color, 1.0f);
		uint8_t r = (uint8_t)(clampedColor.r * 255);
		uint8_t g = (uint8_t)(clampedColor.g * 255);
		uint8_t b = (uint8_t)(clampedColor.b * 255);

		return 0x00000000 | (r << 16) | (g << 8) | b; // Shifting so uint32_t represent RGB
	}

	inline mat4 projectionMatrix(float verticalFov, float nearClip, float farClip, int viewportWidth, int viewportHeight)
	{
		mat4 projMat;
		float halfFov = verticalFov * 0.5f;
		float ctg = cosf(halfFov) / sinf(halfFov);

		projMat[0][0] = (float)viewportHeight / (float)viewportWidth * ctg;
		projMat[1][1] = ctg;
		projMat[2][2] = nearClip / (nearClip - farClip);
		projMat[2][3] = 1;
		projMat[3][2] = -farClip * nearClip / (nearClip - farClip);

		return projMat;
	}

	inline mat4 projectionMatrix(float verticalFov, float nearClip, float farClip, float aspectRatio)
	{
		mat4 projMat;
		float halfFov = verticalFov * 0.5f;
		float ctg = cosf(halfFov) / sinf(halfFov);

		projMat[0][0] = 1.0f / aspectRatio * ctg;
		projMat[1][1] = ctg;
		projMat[2][2] = nearClip / (nearClip - farClip);
		projMat[2][3] = 1;
		projMat[3][2] = -farClip * nearClip / (nearClip - farClip);

		return projMat;
	}

	inline mat4 orthographicProjecton(float right, float left, float top, float bottom, float n, float f)
	{
		mat4 projection;

		projection[0][0] = 2.0f / (right - left);
		projection[1][1] = 2.0f / (top - bottom);
		projection[2][2] = 1.0f / (n - f);
		projection[3][3] = 1.0f;
		
		projection[3][0] = (left + right) / (left - right);
		projection[3][1] = (bottom + top) / (bottom - top);
		projection[3][2] = -f / (n - f);

		return projection;
	}

	inline mat4 transformMatrix(const vec3& position, const vec3& forwardVector, const vec3& rightVector, const vec3& upVector)
	{

		mat4 matrix;

		matrix[0][0] = rightVector.x;
		matrix[0][1] = rightVector.y;
		matrix[0][2] = rightVector.z;

		matrix[1][0] = upVector.x;
		matrix[1][1] = upVector.y;
		matrix[1][2] = upVector.z;

		matrix[2][0] = forwardVector.x;
		matrix[2][1] = forwardVector.y;
		matrix[2][2] = forwardVector.z;

		matrix[3][0] = position.x;
		matrix[3][1] = position.y;
		matrix[3][2] = position.z;
		matrix[3][3] = 1;

		return matrix;
	}

	inline mat4 viewMatrix(const vec3& eye, const vec3& forward, const vec3& upVector) // View matrix for camera like in shooter games
	{
		vec3 rightVector(cross(upVector, forward).normalized());
		vec3 u(cross(forward, rightVector));
		mat4 matrix;

		matrix[0][0] = rightVector.x;
		matrix[1][0] = rightVector.y;
		matrix[2][0] = rightVector.z;

		matrix[0][1] = u.x;
		matrix[1][1] = u.y;
		matrix[2][1] = u.z;
		
		matrix[0][2] = forward.x;
		matrix[1][2] = forward.y;
		matrix[2][2] = forward.z;

		matrix[3][0] = -dot(rightVector, eye);
		matrix[3][1] = -dot(u, eye);
		matrix[3][2] = -dot(forward, eye);
		matrix[3][3] = 1;

		return matrix;
	}

	inline mat4 viewMatrix(const vec3& position, const vec3& forwardVector, const vec3& rightVector, const vec3& upVector)
	{
		mat4 matrix;

		matrix[0][0] = rightVector.x;
		matrix[0][1] = upVector.x;
		matrix[0][2] = forwardVector.x;

		matrix[1][0] = rightVector.y;
		matrix[1][1] = upVector.y;
		matrix[1][2] = forwardVector.y;

		matrix[2][0] = rightVector.z;
		matrix[2][1] = upVector.z;
		matrix[2][2] = forwardVector.z;

		matrix[3][0] = -dot(rightVector, position);
		matrix[3][1] = -dot(upVector, position);
		matrix[3][2] = -dot(forwardVector, position);
		matrix[3][3] = 1;

		return matrix;
	}

	inline mat4 InverseLookAt(vec3 eye, vec3 center, vec3 upVector)
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

	inline mat4 LookAt(vec3 eye, vec3 center, vec3 upVector)
	{
		vec3 f((center - eye).normalized());
		vec3 rightVector(cross(upVector, f).normalized());
		vec3 u(cross(f, rightVector));
		mat4 matrix;

		matrix[0][0] = rightVector.x;
		matrix[1][0] = rightVector.y;
		matrix[2][0] = rightVector.z;

		matrix[0][1] = u.x;
		matrix[1][1] = u.y;
		matrix[2][1] = u.z;

		matrix[0][2] = f.x;
		matrix[1][2] = f.y;
		matrix[2][2] = f.z;

		matrix[3][0] = -dot(rightVector, eye);
		matrix[3][1] = -dot(u, eye);
		matrix[3][2] = -dot(f, eye);
		matrix[3][3] = 1;

		return matrix;
	}

	inline void clamp(vec3& v)
	{
		v.x = v.x < 0.0f ? 0.0f : v.x;
		v.y = v.y < 0.0f ? 0.0f : v.y;
		v.z = v.z < 0.0f ? 0.0f : v.z;
	}

	inline void clamp(vec2& v)
	{
		v.x = v.x < 0.0f ? 0.0f : v.x;
		v.y = v.y < 0.0f ? 0.0f : v.y;
	}

	inline void setTransformPosition(mat4* transform, const vec3& position)
	{
		vec3 transformedPosition = vec4(position, 0.0f) * (*transform);

		for (int i = 0; i < 3; i++)
		{
			(*transform)[3][i] = transformedPosition[i];
		}
	}

	inline float Max(const float& a, const float& b)
	{
		return a > b ? a : b;
	}

	inline float Min(const float& a, const float& b)
	{
		return a < b ? a : b;
	}

	inline void basisFromDir(vec3& right,vec3& top,const vec3& dir)
	{
		float k = 1.0f / Max(1.0f + dir.z, 0.00001f);
		float a = dir.y * k;
		float b = dir.y * a;
		float c = -dir.x * a;
		right = vec3(dir.z + b, c, -dir.x);
		top = vec3(c, 1.0f - b, -dir.y);
	}

	inline vec3 topFromDir(const vec3& dir)
	{
		float k = 1.0f / Max(1.0f + dir.z, 0.00001f);
		float a = dir.y * k;
		float b = dir.y * a;
		float c = -dir.x * a;
		return vec3(c, 1.0f - b, -dir.y);
	}

	inline vec2 screenSpaceToNormalizeScreenSpace(vec2 screenPos, int width, int height)
	{
		vec2 screenCoord(screenPos.x, (float)height - screenPos.y);
		screenCoord.x = (screenCoord.x / (float)width - 0.5f) * 2.0f;
		screenCoord.y = (screenCoord.y / (float)height - 0.5f) * 2.0f;

		return screenCoord;
	}
}