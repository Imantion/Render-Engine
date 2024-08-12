#include "ParticleSystem.h"
#include "Graphics/TransformSystem.h"
#include <random>
#include <cmath>

inline float get_random(float min, float max)
{
	static std::mt19937 eng;
	static std::uniform_real_distribution<> dis(min, max);
	return dis(eng);
}

Engine::Emitter::Emitter(vec3 relativePosition, uint32_t transformId, float spawnRate, float spawnRadius, vec3 particleColor) :
	m_relPosition(relativePosition), m_transformId(transformId), m_spawnRate(spawnRate), m_radius(spawnRadius), m_particleColor(particleColor)
{
	m_particles.resize(spawnRate * m_particleMaxLifeTime);
}

Engine::Emitter::Emitter(const Emitter& emitter) : m_relPosition(emitter.m_relPosition),
	m_transformId(emitter.m_transformId),
	m_particleColor(emitter.m_particleColor),
	m_spawnRate(emitter.m_spawnRate),
	m_radius(emitter.m_radius),
	m_particleMaxLifeTime(emitter.m_particleMaxLifeTime),
	m_accumulatedTime(emitter.m_accumulatedTime),
	m_particleCount(emitter.m_particleCount)
{
	m_particles.resize(emitter.m_particles.size());

	for (size_t i = 0; i < m_particleCount; i++)
	{
		m_particles[i] = emitter.m_particles[i];
	}
}


void Engine::Emitter::Emmit(float deltaTime)
{
	m_accumulatedTime += deltaTime;

	int particlesToSpawn = 0;
	float inverseSpawnRate = 1.0f / m_spawnRate;
	if (m_accumulatedTime > inverseSpawnRate)
	{
		particlesToSpawn = (int)(m_accumulatedTime * inverseSpawnRate);
		m_accumulatedTime = particlesToSpawn * inverseSpawnRate;

		particlesToSpawn = (m_particleCount + particlesToSpawn) >= m_particles.size() ? m_particles.size() - m_particleCount : particlesToSpawn;
	}
	
	auto TS = TransformSystem::Init();
	if (m_particleCount < m_particles.size())
	{
		for (size_t i = m_particleCount; i < m_particleCount + particlesToSpawn; i++)
		{
			vec3 transform = (vec3&)*TS->GetModelTransforms(m_transformId)[0].modelToWold[3] + m_relPosition;
			m_particles[i].position.x = get_random(0.0f, m_radius);
			m_particles[i].position.y = get_random(0.0f, m_radius);
			m_particles[i].position.z = get_random(0.0f, m_radius);
			m_particles[i].position += transform;

			m_particles[i].speed.x = get_random(0.0f, 1.0f);
			m_particles[i].speed.y = 0.4f;
			m_particles[i].speed.z = get_random(0.0f, 1.0f);

			m_particles[i].rgba = vec4(m_particleColor, 0.0f);
			m_particles[i].lifetime = m_particleMaxLifeTime - get_random(0.0f, 2.0f);
			m_particles[i].accumulatedTime = 0.0f;
			m_particles[i].size = vec2(0.1f, 0.1f);
			m_particles[i].rotation = get_random(0.0f, 2.0f * (float)M_PI);
			
			++m_particleCount;
		}
	}
}

void Engine::Emitter::Update(float deltaTime)
{
	for (size_t i = 0; i < m_particles.size(); i++)
	{
		if (m_particles[i].lifetime == -1.0f)
			break;
		m_particles[i].accumulatedTime += deltaTime;
		m_particles[i].size *= (1.0f + m_particles[i].accumulatedTime / m_particleMaxLifeTime);
		m_particles[i].position += m_particles[i].speed * deltaTime;

		if (m_particles[i].accumulatedTime <= m_particles[i].lifetime * 0.1f)
		{
			m_particles[i].rgba.w = std::lerp(0.0f, 1.0f, m_particles[i].accumulatedTime / (m_particles[i].lifetime * 0.1f));
		}
		else if (m_particles[i].accumulatedTime >= m_particles[i].lifetime * 0.9f)
		{
			m_particles[i].rgba.w = std::lerp(1.0f, 0.0f, m_particles[i].accumulatedTime / m_particles[i].lifetime);
		}
		else
			m_particles[i].rgba.w = 1.0f;
	}

	Emmit(deltaTime);
}

void Engine::Emitter::DestroyParticles()
{
	uint32_t diedParticle = m_particleCount;
	uint32_t diedAmount = 0;

	for (size_t i = 0; i < m_particleCount; i++)
	{
		if (m_particles[i].accumulatedTime >= m_particles[i].lifetime)
		{
			diedParticle = i;
			++diedAmount;
			break;
		}
	}

	for (size_t i = diedParticle; i < m_particleCount; i++)
	{
		if (m_particles[i].accumulatedTime < m_particles[i].lifetime)
		{
			m_particles[diedParticle] = m_particles[i];
			++diedParticle;
		}
		else
			++diedAmount;
	}

	m_particleCount -= diedAmount;

	for (size_t i = m_particleCount; i < m_particleCount + diedAmount; i++)
	{
		m_particles[i].lifetime = -1.0f;
	}
	
}

void Engine::ParticleSystem::addSmokeEmmiter(const Emitter& emitter)
{
	m_emmiters.push_back(emitter);
	m_vertexBufferData.resize(m_vertexBufferData.size() + emitter.getMaxSize());
}

void Engine::ParticleSystem::UpdateBuffers(const vec3& cameraPosition)
{
	int size = 0;
	for (size_t i = 0; i < m_emmiters.size(); i++)
	{
		size += m_emmiters[i].getSize();
	}
	m_vertexBufferData.resize(size);

	int index = 0;
	for (size_t i = 0; i < m_emmiters.size(); i++)
	{
		auto& particles = m_emmiters[i].getParticles();
		for (size_t j = 0; j < m_emmiters[i].getMaxSize(); j++)
		{
			if (particles[i].lifetime == -1.0f)
				break;
			m_vertexBufferData[index] = particles[j];
			++index;
		}
	}

	std::sort(m_vertexBufferData.begin(), m_vertexBufferData.end(), [cameraPosition](const Particle& a, const Particle& b) {
		return (a.position - cameraPosition).length_squared() < (b.position - cameraPosition).length_squared(); });

	m_vertexBuffer.create(m_vertexBufferData.data(), m_vertexBufferData.size());
}
