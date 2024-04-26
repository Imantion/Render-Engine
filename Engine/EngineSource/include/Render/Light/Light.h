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
		float specular = powf(Max(dot(halfWay, hInfo.normal), 0.001f), 32);

		return	mat.color * light.color * (light.intensity / (light.position - hInfo.p).length_squared() * (specular + diffuse));
	}

	inline vec3 LightContribution(const PointLight& light, const hitInfo& hInfo, const Material& mat, const vec3& viewVector)
	{
		vec3 lightDirection = (hInfo.p - light.position).normalized();

		vec3 halfWay = (viewVector - lightDirection).normalized();

		float diffuse = Max(dot(-lightDirection, hInfo.normal), 0.001f);
		float specular = powf(Max(dot(halfWay, hInfo.normal), 0.001f), 32);

		return	mat.color * light.color * (light.intensity / (light.position - hInfo.p).length_squared() * (specular + diffuse));
	}

	inline vec3 LightContribution(const DirectionalLight& light, const hitInfo& hInfo, const Material& mat, const vec3& viewVector)
	{
		float d = Max(dot(-light.direction, hInfo.normal), 0.001f);
		vec3 halfwayVector = (viewVector - light.direction).normalized();
		float specAmount = powf(Max(dot(halfwayVector, hInfo.normal), 0.001f), 32);
		specAmount *= 0.5f;

		return mat.color * light.color *  (d + specAmount);
	}
}