#pragma once
#include "Math/vec.h"

namespace Engine
{
	class Material
	{
	public:
		vec3 color;
		float glossiness;

		Material(): color(0.0f), glossiness(0.0f) {}
		Material(float r, float g, float b) : color(r, g ,b), glossiness(0.0f) {}
		Material(float r, float g, float b, float glossiness) : Material(r, g, b) { this->glossiness = glossiness; }
		Material(const vec3& c) : color(c), glossiness(0.0f) {};
		Material(const vec3& c, float glossiness) : color(c), glossiness(glossiness) {};
		Material(const Material& c) : color(c.color), glossiness(c.glossiness) {}

	};
}