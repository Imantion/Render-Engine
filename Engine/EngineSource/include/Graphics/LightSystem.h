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
	#define MAX_AREA_LIGHTS 1
	#define MAX_AREA_LIGHTS_VERTICES 4
	#define MAX_AREA_LIGHTS_EDGES 4


	class Texture;
	class Model;

	class  Light
	{
	public:

		Light() : radiance(0.0f) {}
		Light(const vec3& radiance) : radiance(radiance) {}

		static vec3 radianceFromIrradiance(const vec3& irradiance, float radius, float distanceSquared)
		{
			return irradiance / (1 - sqrt(1 - Min(radius * radius / distanceSquared, 1.0f)));
		}

	public:
		vec3 radiance;
	};

	class DirectionalLight : public Light
	{
	public:
		DirectionalLight() : Light() {}
		DirectionalLight(const vec3& direction, const vec3& radiance, float solidAngle) : Light(radiance), solidAngle(solidAngle), direction(direction) {}
		float solidAngle;
		vec3 direction;
		int padding;
	};

	class PointLight : public Light
	{
	public:
		PointLight() : Light() {}
		PointLight(const vec3& irradiance, float radius, float distance, const vec3& pos) : radius(radius), position(pos)
		{
			radiance = radianceFromIrradiance(irradiance, radius, distance * distance);
		}

		float radius;
		vec3 position;
		int bindedObjectId = -1;
	};

	class SpotLight : public Light
	{
	public:
		SpotLight() : Light() { cutoffAngle = 0.0f; }
		SpotLight(const vec3& irradiance, float radius, float distance, const vec3& pos, const vec3& direction, float cutoffAngle) : radius(radius), position(pos)
		{
			this->direction = direction;
			this->cutoffAngle = cutoffAngle;
			radiance = radianceFromIrradiance(irradiance, radius, distance * distance);
		}
		float radius;
		vec3 direction;
		float cutoffAngle;
		vec3 position;
		int bindedObjectId = -1;
	};

	class AreaLight
	{
	public:
		AreaLight() = default;
		AreaLight(const vec3& radiance, const vec3* vertices, uint32_t vAmount, float intensity)
		{
			this->radiance = radiance;
			this->verticesAmount = vAmount;
			this->indicesAmount = vAmount;
			this->intensity = intensity;

			for (uint32_t i = 0; i < vAmount; i++)
			{
				this->vertices[i] = vec4(vertices[i], 1.0f);
				this->edges[i].x = i;
				this->edges[i].y = (i + 1) % vAmount;
			}

		}
		AreaLight(const vec3* vertices, uint32_t vAmount, const std::pair<uint32_t, uint32_t>* edges, uint32_t iAmount, float intensity)
		{
			this->radiance = radiance;
			this->verticesAmount = vAmount;
			this->indicesAmount = iAmount;
			this->intensity = intensity;

			for (uint32_t i = 0; i < vAmount; i++)
			{
				this->vertices[i] = vec4(vertices[i],1.0f);
			}

			for (uint32_t i = 0; i < iAmount; i++)
			{
				this->edges[i].x = edges[i].first;
				this->edges[i].y = edges[i].second;
			}
		}

		AreaLight(const AreaLight& other)
		{
			radiance = other.radiance;
			verticesAmount = other.verticesAmount;
			indicesAmount = other.indicesAmount;
			bindedTransform = other.bindedTransform;
			intensity = other.intensity;

			for (size_t i = 0; i < verticesAmount; i++)
			{
				vertices[i] = other.vertices[i];
			}

			for (size_t i = 0; i < indicesAmount; i++)
			{
				edges[i] = other.edges[i];
			}

		}

		void bindTransform(uint32_t id) { bindedTransform = id; }

	public:
		vec3 radiance;
		uint32_t verticesAmount;
		vec4 vertices[MAX_AREA_LIGHTS_VERTICES];
		struct edge { uint32_t x, y; uint32_t padding[2];} edges[MAX_AREA_LIGHTS_EDGES];
		uint32_t indicesAmount;
		float intensity;
		uint32_t bindedTransform;
		float padding;
	};


	class LightSystem
	{
	public:

		static LightSystem* Init();
		static void Deinit();

		void operator=(const LightSystem& other) = delete;
		LightSystem(const LightSystem& other) = delete;

		void AddFlashLight(const SpotLight& spotLight, std::shared_ptr<Texture> texture, float nearCLip = 0.01f, float farClip = 100.0f); // near and far clip for projection matrix


		void AddDirectionalLight(const vec3& direction, const vec3& radiance, float solidAngle);
		void AddDirectionalLight(const DirectionalLight& othe);

		uint32_t AddPointLight(const vec3& irradiance, float radius,float distance, const vec3& pos, std::shared_ptr<Model> model);
		void AddPointLight(const vec3& irradiance, float radius,float distance, const vec3& pos, int objectToBindId);
		void AddPointLight(const PointLight& pointLight);

		uint32_t AddSpotLight(const vec3& irradiance, float radius, float distance, const vec3& pos, const vec3& direction, float cutoffAngle, std::shared_ptr<Model> model);
		void AddSpotLight(const vec3& irradiance, float radius, float distance, const vec3& pos, const vec3& direction, float cutoffAngle, int objectToBindID);
		void AddSpotLight(const SpotLight& spotLight);

		void AddAreaLight(const AreaLight& areaLight);

		SpotLight& GetSpotLight(uint32_t index);
		SpotLight* GetSpotLightByTransformId(uint32_t index);
		std::vector<SpotLight> GetSpotLights();

		PointLight* GetPointLightByTransformId(uint32_t index);
		void GetPointLightsPositions(std::vector<vec3>& positions);
		void GetPointLightsRadius(std::vector<float>& radiuses);

		void UpdateLightsBuffer();
		void BindLigtsBuffer(UINT slot, UINT typeOfShader);
		void BindLightTextures();

		void SetFlashLightAttachedState(bool attach);
		bool IsFlashLightAttached() const { return m_flashLight.light.bindedObjectId != -1; }
		SpotLight getFlashLight() const { return m_flashLight.light; }
		mat4 getFlashLightViewProjection() const { return m_flashLight.flashLightsViewProjection; }

		std::vector<DirectionalLight> GetDirectionalLights();

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
			int lastBindedTransform = -1;

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
		std::vector <AreaLight> m_areaLight;
		std::vector <mat4> m_spotLigtsViewProjection;

		struct LightsData
		{
			AreaLight areaLights[MAX_AREA_LIGHTS];
			DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
			PointLight pointLights[MAX_POINT_LIGHTS];
			SpotLight spotLights[MAX_SPOT_LIGHTS];
			SpotLight flashLight;
			mat4 flashLightsViewProjection;
			UINT dlSize = 0;
			UINT plSize = 0;
			UINT spSize = 0;
			UINT alSize = 0;
		};

		ConstBuffer<LightsData> m_lighsBuffer;
	};


}