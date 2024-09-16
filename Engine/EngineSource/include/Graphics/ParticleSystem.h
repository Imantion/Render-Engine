#pragma once
#include <vector>
#include "Graphics/D3D.h"
#include "Graphics/Buffers.h"
#include "Math\math.h"
#include "Utils/Definitions.h"
#include <mutex>
#include <memory>


namespace Engine
{
	class Texture;
	struct shader;
	struct IndexedDistance;

	struct Particle
	{
		vec4 rgba;
		vec3 position;
		float rotation;
		vec3 speed;
		vec2 size;

		float accumulatedTime = 0.0f;
		float lifetime = -1.0f;
	};

	class Emitter
	{
	public:
		Emitter(vec3 relativePosition, uint32_t transformId, float spawnRate, float spawnRadius, vec3 particleColor);
		Emitter(const Emitter& emitter);

		void Update(float deltaTime);
	
		uint32_t getMaxSize() const { return (uint32_t)m_particles.size(); }
		uint32_t getSize() const { return m_particleCount; }
		const std::vector<Particle>& getParticles() const { return m_particles; }
		uint32_t getBindedTransformId() const { return m_transformId; }

		void adjustParameters(const vec3& maxSpeed, const vec2& size, float spawnRate, float lifetime);
		void setColor(const vec3& color);

		vec2 getParticleSize() { return m_size; }
		vec3 getParticleMaxSpeed() { return m_maxSpeed; }
		float getParticleSpawnRate() { return m_spawnRate; }
		float getParticleLifeTime() { return m_particleMaxLifeTime; }
		vec3 getParticleColor() { return m_particleColor; }

	private:
		void DestroyParticles();
		void Emmit(float deltaTime);

	private:
		vec3 m_relPosition;
		uint32_t m_transformId;
		vec3 m_particleColor;
		float m_spawnRate;
		float m_radius;
		std::vector<Particle> m_particles;

		float m_particleMaxLifeTime = 10.0f;
		float m_accumulatedTime = 0.0f;
		vec2 m_size = vec2(0.5f);
		vec3 m_maxSpeed = vec3(0.5f, 0.1f, 0.5f);
		uint32_t m_particleCount = 0;
	};

	class ParticleSystem
	{
	public:

		ParticleSystem(const ParticleSystem& other) = delete;
		ParticleSystem& operator=(const ParticleSystem& other) = delete;

		static ParticleSystem* Init();
		static void Deinit();

		void addSmokeEmitter(const Emitter& emitter);
		Emitter* getEmitterByTransformId(uint32_t id);
		void Update(float deltaTime);
		void UpdateBuffers(const vec3& cameraPosition);

		void SetSmokeTextures(std::shared_ptr<Texture> RLU, std::shared_ptr<Texture> DBF, std::shared_ptr<Texture> EMVA);
		void Render();
		

	private:
		ParticleSystem();

		struct ParticleInstance
		{
			vec4 rgba;
			vec3 position;
			float rotation;
			vec2 size;
			float frameFraction;
			uint32_t frameIndex;
		};

		struct PartilcleAtlasInfo
		{
			uint32_t rows;
			uint32_t columns;
			uint32_t padding[2];
		};

	private:
		VertexBuffer<ParticleInstance> m_vertexBuffer;
		std::vector<ParticleInstance> m_vertexBufferData;
		std::vector<ParticleInstance> m_vertexBufferSortedData;
		
		std::vector<IndexedDistance> m_indexedDistance;
		std::vector<IndexedDistance> m_sortedIndexedDistance;

		uint32_t particlesAmount = 0;

		IndexBuffer m_indexBuffer;
		ConstBuffer<PartilcleAtlasInfo> m_cbTextureData;

		std::vector<Emitter> m_emmiters;

		std::shared_ptr<Texture> m_RLU;
		std::shared_ptr<Texture> m_DBF;
		std::shared_ptr<Texture> m_EMVA;

		uint32_t textureColumnCount = 8;
		uint32_t textureRowCount = 8;
		uint32_t textureFrameCount;

		std::shared_ptr<shader> m_shader;
	private:

		static std::mutex m_mutex;
		static ParticleSystem* m_instance;
		
	};
}
