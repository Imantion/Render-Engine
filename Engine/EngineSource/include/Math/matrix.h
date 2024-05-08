#pragma once
#include <memory>
#include "Math/vec.h"

namespace Engine
{
	struct vec4;

	struct mat4
	{
		mat4() : mat4(0.0f) {}
		mat4(float t);
		mat4(vec4 x, vec4 y, vec4 z, vec4 w);
		mat4(const mat4& other);

		static mat4 Inverse(const mat4& m);

		
		mat4& operator=(const mat4& other);
		float* operator[](int index) { return mat[index]; }
		const float* operator[](int index) const { return mat[index]; } 
		mat4 operator*(float t);
		mat4 operator*(const mat4& other) const;
		

		float mat[4][4];
	};	
}

inline Engine::vec4 operator*(const Engine::vec4& v, const Engine::mat4& m)
{
	Engine::vec4 product;
	product.x = v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + v.w * m[3][0];
	product.y = v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + v.w * m[3][1];
	product.z = v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + v.w * m[3][2];
	product.w = v.x * m[0][3] + v.y * m[1][3] + v.z * m[2][3] + v.w * m[3][3];

	return product;
}

