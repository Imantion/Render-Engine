#pragma once
#include "Math\vec.h"
#include "Math\matrix.h"

namespace Engine
{

	struct quaternion
	{ 
		quaternion() : r(0.0f), im(0.0f){}
		quaternion(float real, vec3 imagine) : r(real), im(imagine) {}

		quaternion conjugate() const { return quaternion(r, -im); }
		float norm() const;
		float norm_squared() const;
		quaternion normalize() const;
		mat4 toMat4();
		static quaternion angleAxis(float angle, vec3 axis);
		static vec3 rotate(quaternion q, vec3 v) { return q * v; }

		quaternion operator+(const quaternion& q) const;
		quaternion operator-(const quaternion& q) const;
		quaternion operator*(const quaternion& q) const;
		vec3 operator*(const vec3& q) const;
		quaternion operator*(const float t) const;
		
		
		static quaternion slerp(quaternion q1, quaternion q2, double lambda);

		float r;
		vec3 im;
	};
}