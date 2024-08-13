#pragma once
#include <vector>
#include "Graphics/D3D.h"
#include "Graphics/Buffers.h"
#include "Math\math.h"
#include <mutex>
#include <memory>

namespace Engine
{
	class Texture;
	struct shader;

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

	private:
		void DestroyParticles();
		void Emmit(float deltaTime);

	private:
		vec3 m_relPosition;
		uint32_t m_transformId;
		vec3 m_particleColor;
		float m_spawnRate;
		float m_radius;
		float m_particleMaxLifeTime = 10.0f;
		float m_accumulatedTime = 0.0f;
		std::vector<Particle> m_particles;
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

	private:
		VertexBuffer<ParticleInstance> m_vertexBuffer;
		std::vector<ParticleInstance> m_vertexBufferData;
		IndexBuffer m_indexBuffer;
		ConstBuffer<vec4> m_cbTextureData;

		std::vector<Emitter> m_emmiters;

		std::shared_ptr<Texture> m_RLU;
		std::shared_ptr<Texture> m_DBF;
		std::shared_ptr<Texture> m_EMVA;

		uint32_t textureFrameCount = 64;

		std::shared_ptr<shader> m_shader;
	private:

		static std::mutex m_mutex;
		static ParticleSystem* m_instance;
		
	};
}
