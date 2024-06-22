#pragma once
#include "Math/vec.h"
#include "Math/math.h"
#include <math.h>

namespace Engine
{
	class  Light
	{
	public:

		Light() : color(0.0f) {}
		Light(const vec3& col) : color(col) {}

	public:
		vec3 color;
	};

	class DirectionalLight : public Light
	{
	public:
		vec3 direction;

		DirectionalLight() : Light() {}
		DirectionalLight(const vec3& direction, const vec3& color) : Light(color), direction(direction) {}
	};

	class PointLight : public Light
	{
	public:
		PointLight() : Light() {}
		PointLight(const vec3& col, const vec3& pos, float intens) : Light(col), position(pos), intensity(intens) {};
		vec3 position;
		float intensity;
	};

	class SpotLight : public Light
	{
	public:
		SpotLight() : Light() { cutoffAngle = 0.0f;}
		SpotLight(const vec3& col, const vec3& pos, const vec3& direction, float cutoffAngle, float intens) : Light(col), position(pos), intensity(intens)
		{
			this->direction = direction;
			this->cutoffAngle = cutoffAngle;
		}
		float cutoffAngle; // in Radians
		vec3 direction;
		vec3 position;
		float intensity;
	};

}