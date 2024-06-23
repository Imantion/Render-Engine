#pragma once
#include <mutex>
#include <math.h>
#include <vector>
#include "Math/vec.h"
#include "Math/math.h"
#include "Buffers.h"

namespace Engine
{
	#define MAX_SPOTLIGHTS 10
	#define MAX_POINTLIGHTS 10
	#define MAX_DIRECTIONALLIGHTS 1
	class  Light
	{
	public:

		Light() : color(0.0f), intensity(0.0f) {}
		Light(const vec3& col, float intensity) : color(col), intensity(intensity) {}

	public:
		vec3 color;
		float intensity;
	};

	class DirectionalLight : public Light
	{
	public:
		DirectionalLight() : Light() {}
		DirectionalLight(const vec3& direction, const vec3& color, float intens) : Light(color, intens), direction(direction) {}
		vec3 direction;
		int padding;
	};

	class PointLight : public Light
	{
	public:
		PointLight() : Light() {}
		PointLight(const vec3& col, const vec3& pos, float intens) : Light(col, intens), position(pos) {};
		vec3 position;
		int bindedObjectId  = -1;
	};

	class SpotLight : public Light
	{
	public:
		SpotLight() : Light() { cutoffAngle = 0.0f;}
		SpotLight(const vec3& col, const vec3& pos, const vec3& direction, float cutoffAngle, float intens) : Light(col, intens), position(pos)
		{
			this->direction = direction;
			this->cutoffAngle = cutoffAngle;
		}
		vec3 direction;
		float cutoffAngle; // in Radians
		vec3 position;
		int bindedObjectId = -1;
	};


	class LightSystem
	{
	public:

		static LightSystem* Init();
		static void Deinit();

		void operator=(const LightSystem& other) = delete;
		LightSystem(const LightSystem& other) = delete;

		void AddDirectionalLight(const vec3& direction, const vec3& color, float intensity);
		void AddDirectionalLight(const DirectionalLight& othe);

		void AddPointLight(const vec3& col, const vec3& pos, float intens, int objectToBindId);
		void AddPointLight(const PointLight& pointLight);

		void AddSpotLight(const vec3& col, const vec3& pos, const vec3& direction, float cutoffAngle, float intens, int objectToBindID);
		void AddSpotLight(const SpotLight& spotLight);

		void UpdateLightsBuffer();
		void BindLigtsBuffer(UINT slot, UINT typeOfShader);


	private:
		LightSystem();

	private:
		static LightSystem* m_instance;
		static std::mutex m_mutex;

		std::vector <DirectionalLight> m_directionalLights;
		std::vector <PointLight> m_pointLights;
		std::vector <SpotLight> m_spotLights;

		struct LightsData
		{
			DirectionalLight directionalLights[MAX_DIRECTIONALLIGHTS];
			PointLight pointLights[MAX_POINTLIGHTS];
			SpotLight spotLights[MAX_SPOTLIGHTS];
			int dlSize = 0;
			int plSize = 0;
			int spSize = 0;
			int padding;
		};

		ConstBuffer<LightsData> m_lighsBuffer;
	};


}