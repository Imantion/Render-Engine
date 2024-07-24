#pragma once
#include <mutex>
#include <math.h>
#include <vector>
#include "Math/vec.h"
#include "Math/math.h"
#include "Buffers.h"

namespace Engine
{
	#define MAX_SPOT_LIGHTS 10
	#define MAX_POINT_LIGHTS 10
	#define MAX_DIRECTIONAL_LIGHTS 1

	class Texture;

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
		float cutoffAngle;
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

		void AddFlashLight(const SpotLight& spotLight, std::shared_ptr<Texture> texture, float aspectRatio = 1.0f, float nearCLip = 0.01f, float farClip = 10.0f);


		void AddDirectionalLight(const vec3& direction, const vec3& color, float intensity);
		void AddDirectionalLight(const DirectionalLight& othe);

		void AddPointLight(const vec3& col, const vec3& pos, float intens, int objectToBindId);
		void AddPointLight(const PointLight& pointLight);

		void AddSpotLight(const vec3& col, const vec3& pos, const vec3& direction, float cutoffAngle, float intens, int objectToBindID);
		void AddSpotLight(const SpotLight& spotLight);

		SpotLight& GetSpotLight(uint32_t index);

		void UpdateLightsBuffer();
		void BindLigtsBuffer(UINT slot, UINT typeOfShader);
		void BindLightTextures();

		void SetFlashLightAttachedState(bool attach);
		bool IsFlashLightAttached() const { return m_flashLight.isAttached; }


	private:
		LightSystem();

	private:
		static LightSystem* m_instance;
		static std::mutex m_mutex;

		struct FlashLight
		{
			SpotLight light;
			vec3 worldPosition;
			vec3 worldDirection;
			std::shared_ptr<Texture> flashLightMask;
			mat4 flashLightsViewProjection;
			bool isAttached = false;

		} m_flashLight;

		struct FlashLightProjectionData
		{
			float aspectRatio;
			float nearClip;
			float farClip;
		} flProjectionData;

		std::vector <DirectionalLight> m_directionalLights;
		std::vector <PointLight> m_pointLights;
		std::vector <SpotLight> m_spotLights;
		std::vector <mat4> m_spotLigtsViewProjection;

		struct LightsData
		{
			DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
			PointLight pointLights[MAX_POINT_LIGHTS];
			SpotLight spotLights[MAX_SPOT_LIGHTS];
			SpotLight flashLight;
			mat4 flashLightsViewProjection;
			UINT dlSize = 0;
			UINT plSize = 0;
			UINT spSize = 0;
			UINT padding;
		};

		ConstBuffer<LightsData> m_lighsBuffer;
	};


}