#include "Math/matrix.h"
#include "Math/vec.h"
#include <cmath>
Engine::mat4::mat4(float t)
{
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			mat[i][j] = t;
		}
	}
}


Engine::mat4::mat4(vec4 x, vec4 y, vec4 z, vec4 w)
{
		mat[0][0] = x.x; mat[0][1] = x.y; mat[0][2] = x.z; mat[0][3] = x.w;
		mat[1][0] = y.x; mat[1][1] = y.y; mat[1][2] = y.z; mat[1][3] = y.w;
		mat[2][0] = z.x; mat[2][1] = z.y; mat[2][2] = z.z; mat[2][3] = z.w;
		mat[3][0] = w.x; mat[3][1] = w.y; mat[3][2] = w.z; mat[3][3] = w.w;
}

Engine::mat4::mat4(const mat4& other)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			mat[i][j] = other[i][j];
		}
	}
}


Engine::mat4 Engine::mat4::Inverse(const mat4& m)
{
	float Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
	float Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
	float Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];
	float Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
	float Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
	float Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];
	float Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
	float Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
	float Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];
	float Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
	float Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
	float Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];
	float Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
	float Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
	float Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];
	float Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
	float Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
	float Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];
	vec4 Fac0(Coef00, Coef00, Coef02, Coef03);
	vec4 Fac1(Coef04, Coef04, Coef06, Coef07);
	vec4 Fac2(Coef08, Coef08, Coef10, Coef11);
	vec4 Fac3(Coef12, Coef12, Coef14, Coef15);
	vec4 Fac4(Coef16, Coef16, Coef18, Coef19);
	vec4 Fac5(Coef20, Coef20, Coef22, Coef23);
	vec4 Vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
	vec4 Vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
	vec4 Vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
	vec4 Vec3(m[1][3], m[0][3], m[0][3], m[0][3]);
	vec4 Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
	vec4 Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
	vec4 Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
	vec4 Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);
	vec4 SignA(+1, -1, +1, -1);
	vec4 SignB(-1, +1, -1, +1);
	mat4 inverse(Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB);
	vec4 Row0(inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0]);
	vec4 Dot0(m[0][0] * Row0.x, m[0][1] * Row0.y, m[0][2] * Row0.z, m[0][3] * Row0.w);
	float Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);
	float OneOverDeterminant = 1.0f / Dot1;
	return inverse * OneOverDeterminant;
}

Engine::mat4 Engine::mat4::rotateX(float angle)
{
	mat4 result;
	float sinTheta = std::sinf(angle);
	float cosTheta = std::cosf(angle);

	result.mat[0][0] = 1.0f;
	result.mat[1][1] = cosTheta;
	result.mat[1][2] = -sinTheta;
	result.mat[2][1] = sinTheta;
	result.mat[2][2] = cosTheta;
	result.mat[3][3] = 1.0f;

	return result;
}

Engine::mat4 Engine::mat4::rotateY(float angle)
{
	mat4 result;
	float sinTheta = std::sinf(angle);
	float cosTheta = std::cosf(angle);

	result.mat[0][0] = cosTheta;
	result.mat[0][2] = sinTheta;
	result.mat[1][1] = 1.0f;
	result.mat[2][0] = -sinTheta;
	result.mat[2][2] = cosTheta;
	result.mat[3][3] = 1.0f;

	return result;
}

Engine::mat4 Engine::mat4::rotateZ(float angle)
{
	mat4 result;
	float sinTheta = std::sinf(angle);
	float cosTheta = std::cosf(angle);

	result.mat[0][0] = cosTheta;
	result.mat[0][1] = -sinTheta;
	result.mat[1][0] = sinTheta;
	result.mat[1][1] = cosTheta;
	result.mat[2][2] = 1.0f;
	result.mat[3][3] = 1.0f;

	return result;
}

Engine::mat4& Engine::mat4::operator=(const mat4& other)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			mat[i][j] = other[i][j];
		}
	}

	return *this;
}

Engine::mat4 Engine::mat4::operator*(float t)
{
	mat4 ret;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			ret[i][j] = mat[i][j] * t;
		}
	}

	return ret;
}

Engine::mat4 Engine::mat4::operator*(const mat4& m2) const
{
	Engine::mat4 result;

	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result[row][col] = mat[row][0] * m2[0][col] +
				mat[row][1] * m2[1][col] +
				mat[row][2] * m2[2][col] +
				mat[row][3] * m2[3][col];
		}
	}

	return result;
}
