#include "Math\quaternion.h"
#include "Math\math.h"
#include <math.h>

float Engine::quaternion::norm() const
{
	return sqrtf(r * r + im.length_squared());
}

float Engine::quaternion::norm_squared() const
{
	return r * r + im.length_squared();
}

Engine::quaternion Engine::quaternion::normalize() const
{
	float q_norm = norm();
	return quaternion(r / q_norm, im / q_norm);
}

Engine::quaternion Engine::quaternion::angleAxis(float angle, vec3 axis)
{
	float half_angle = angle * 0.5f;
	return quaternion(cosf(half_angle), axis * sinf(half_angle));
}

Engine::quaternion Engine::quaternion::operator+(const quaternion& q) const
{
	return quaternion(r + q.r, im + q.im);
}

Engine::quaternion Engine::quaternion::operator-(const quaternion& q) const
{
	return quaternion(r - q.r, im - q.im);
}

Engine::quaternion Engine::quaternion::operator*(const quaternion& q) const
{
	quaternion quat;
	quat.r = r * q.r - im.x * q.im.x - im.y * q.im.y - im.z * q.im.z;
	quat.im.x = r * q.im.x + q.r * im.x + im.y * q.im.z - q.im.y * im.z;
	quat.im.y = r * q.im.y + q.r * im.y + im.z * q.im.x - q.im.z * im.x;
	quat.im.z = r * q.im.z + q.r * im.z + im.x * q.im.y - q.im.x * im.y;

	return quat;
}

Engine::vec3 Engine::quaternion::operator*(const vec3& v) const
{
	vec3 const QuatVector(im.x, im.y,im.z);
	vec3 const uv(cross(QuatVector, v));
	vec3 const uuv(cross(QuatVector, uv));

	return v + ((uv * r) + uuv) * 2.0f;
}

Engine::quaternion Engine::quaternion::operator*(const float t) const
{
	return quaternion(r * t, im * t);
}
