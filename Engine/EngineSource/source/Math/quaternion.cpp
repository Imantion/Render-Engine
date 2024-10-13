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

Engine::mat4 Engine::quaternion::toMat4()
{
	float x = this->im.x;
	float y = this->im.y;
	float z = this->im.z;
	float w = this->r;

	return mat4(
		1.0f - 2.0f * (y * y + z * z), 2.0f * (x * y + w * z), 2.0f * (x * z - w * y), 0.0f,
		2.0f * (x * y - w * z), 1.0f - 2.0f * (x * x + z * z), 2.0f * (y * z + w * x), 0.0f,
		2.0f * (x * z + w * y), 2.0f * (y * z - w * x), 1.0f - 2.0f * (x * x + y * y), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

Engine::quaternion Engine::quaternion::slerp(quaternion q1, quaternion q2, double lambda)
{
	quaternion qr;
	float dotproduct = q1.im.x * q2.im.x + q1.im.y * q2.im.y + q1.im.z * q2.im.z + q1.r * q2.r;
	float theta, st, sut, sout, coeff1, coeff2;

	// algorithm adapted from Shoemake's paper
	lambda = lambda / 2.0;

	theta = (float)acos(dotproduct);
	if (theta < 0.0) theta = -theta;

	st = (float)sin(theta);
	sut = (float)sin(lambda * theta);
	sout = (float)sin((1 - lambda) * theta);
	coeff1 = sout / st;
	coeff2 = sut / st;

	qr.im.x = coeff1 * q1.im.x + coeff2 * q2.im.x;
	qr.im.y = coeff1 * q1.im.y + coeff2 * q2.im.y;
	qr.im.z = coeff1 * q1.im.z + coeff2 * q2.im.z;
	qr.r = coeff1 * q1.r + coeff2 * q2.r;

	return qr.normalize();
}