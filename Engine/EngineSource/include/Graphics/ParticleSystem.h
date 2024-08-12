#pragma once
#include <vector>
#include "Graphics/D3D.h"
#include "Graphics/Buffers.h"
#include "Math\math.h"

namespace Engine
{
	struct Particle
	{
		vec4 rgba;
		vec3 position;
		float rotation;
		vec3 speed;
		vec2 size;

		float accumulatedTime;
		float lifetime;
	};

	class Emitter
	{
	public:
		Emitter(vec3 relativePosition, uint32_t transformId, float spawnRate, float spawnRadius, vec3 particleColor);
		Emitter(const Emitter& emitter);

		void Emmit(float deltaTime);
		void Update(float deltaTime);
		void DestroyParticles();

		uint32_t getMaxSize() const { return m_particles.size(); }
		uint32_t getSize() const { return m_particleCount; }
		const std::vector<Particle>& getParticles() const { return m_particles; }

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

		void addSmokeEmmiter(const Emitter& emitter);

		void UpdateBuffers(const vec3& cameraPosition);

	private:
		VertexBuffer<Particle> m_vertexBuffer;
		std::vector<Particle> m_vertexBufferData;
		IndexBuffer m_indexBuffer;

		std::vector<Emitter> m_emmiters;
		
	};
}
