#pragma once
#include "Math/vec.h"
#include "Math/math.h"
#include <math.h>

namespace Engine
{
	class  Light
	{
	public:

		Light() : color(0.0f), position(0.0f), intensity(0.0f) {}
		Light(vec3 col, vec3 pos, float intens) : color(col), position(pos), intensity(intens) {}
		vec3 color;
		vec3 position;
		float intensity;
	};

	class PointLight : public Light
	{
	public:
		PointLight() : Light() {}
		PointLight(vec3 col, vec3 pos, float intens) : Light(col, pos, intens) {};
	};

	class SpotLight : public Light
	{
	public:
		SpotLight() : Light() { cutoffAngle = 0.0f;}
		SpotLight(vec3 col, vec3 pos, vec3 direction, float cutoffAngle, float intens) : Light(col, pos, intens)
		{
			this->direction = direction;
			this->cutoffAngle = cutoffAngle;
		}
		float cutoffAngle; // in Radians
		vec3 direction;
	};

	inline vec3 LightContribution(const SpotLight& light, const hitInfo& hInfo, const Material& mat, const vec3& viewVector)
	{
		vec3 lightDirection = (hInfo.p - light.position).normalized();

		float cosAngle = dot(lightDirection, light.direction);
		if (cosAngle < cos(light.cutoffAngle))
		{
			return vec3(0.001f) * mat.color;
		}

		vec3 halfWay = (viewVector - lightDirection).normalized();

		float diffuse = Max(dot(-lightDirection, hInfo.normal), 0.001f);
		float specular = pow(Max(dot(halfWay, hInfo.normal), 0.001f), 32);

		return	mat.color * light.color * (light.intensity / (light.position - hInfo.p).length_squared() * (specular + diffuse));
	}

	inline vec3 LightContribution(const PointLight& light, const hitInfo& hInfo, const Material& mat, const vec3& viewVector)
	{
		vec3 lightDirection = (hInfo.p - light.position).normalized();

		vec3 halfWay = (viewVector - lightDirection).normalized();

		float diffuse = Max(dot(-lightDirection, hInfo.normal), 0.001f);
		float specular = pow(Max(dot(halfWay, hInfo.normal), 0.001f), 32);

		return	mat.color * light.color * (light.intensity / (light.position - hInfo.p).length_squared() * (specular + diffuse));
	}
}