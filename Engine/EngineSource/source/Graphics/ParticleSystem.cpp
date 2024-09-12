#include "Graphics/ParticleSystem.h"
#include "Graphics/TransformSystem.h"
#include "Graphics/TextureManager.h"
#include "Graphics/ShaderManager.h"
#include "Utils/Random.h"
#include "Utils/RadixSort.h"
#include <cmath>



Engine::Emitter::Emitter(vec3 relativePosition, uint32_t transformId, float spawnRate, float spawnRadius, vec3 particleColor) :
	m_relPosition(relativePosition), m_transformId(transformId), m_spawnRate(spawnRate), m_radius(spawnRadius), m_particleColor(particleColor)
{
	m_particles.resize((size_t)ceil(spawnRate * m_particleMaxLifeTime));
}

Engine::Emitter::Emitter(const Emitter& emitter) : m_relPosition(emitter.m_relPosition),
	m_transformId(emitter.m_transformId),
	m_particleColor(emitter.m_particleColor),
	m_spawnRate(emitter.m_spawnRate),
	m_radius(emitter.m_radius),
	m_particleMaxLifeTime(emitter.m_particleMaxLifeTime),
	m_accumulatedTime(emitter.m_accumulatedTime),
	m_particleCount(emitter.m_particleCount),
	m_maxSpeed(emitter.m_maxSpeed),
	m_size(emitter.m_size)
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
		particlesToSpawn = (int)(m_accumulatedTime * m_spawnRate);
		m_accumulatedTime -= particlesToSpawn * inverseSpawnRate;

		particlesToSpawn = (int)((m_particleCount + particlesToSpawn) >= m_particles.size() ? m_particles.size() - m_particleCount : particlesToSpawn);
	}
	
	auto TS = TransformSystem::Init();
	if (m_particleCount < m_particles.size())
	{
		for (size_t i = m_particleCount; i < m_particleCount + particlesToSpawn; i++)
		{
			vec3 transform = (vec3&)*TS->GetModelTransforms(m_transformId)[0].modelToWold[3] + m_relPosition;
			m_particles[i].position.x = get_random(g_distribution_neg1_1) * m_radius;
			m_particles[i].position.y = 0;
			m_particles[i].position.z = get_random(g_distribution_neg1_1) * m_radius;
			m_particles[i].position += transform;

			m_particles[i].speed.x = get_random(g_distribution_neg1_1) * m_maxSpeed.x;
			m_particles[i].speed.y = m_maxSpeed.y;
			m_particles[i].speed.z = get_random(g_distribution_neg1_1) * m_maxSpeed.z;

			m_particles[i].rgba = vec4(m_particleColor, 0.0f);
			m_particles[i].lifetime = m_particleMaxLifeTime - get_random(g_distribution_neg1_1);
			m_particles[i].accumulatedTime = 0.0f;
			m_particles[i].size = m_size;
			m_particles[i].rotation = get_random(g_distribution_neg1_1);
			
		}

		m_particleCount += particlesToSpawn;
	}
}

void Engine::Emitter::Update(float deltaTime)
{
	Emmit(deltaTime);

	for (size_t i = 0; i < m_particleCount; i++)
	{
		m_particles[i].accumulatedTime += deltaTime;
		m_particles[i].position += m_particles[i].speed * deltaTime;
		float dissappearFactor = m_particles[i].lifetime * 0.8f;

		if (m_particles[i].accumulatedTime <= m_particles[i].lifetime * 0.1f)
		{
			m_particles[i].rgba.w = std::lerp(0.0f, 1.0f, m_particles[i].accumulatedTime / (m_particles[i].lifetime * 0.1f));
		}
		else if (m_particles[i].accumulatedTime >= dissappearFactor)
		{
			m_particles[i].rgba.w = std::lerp(1.0f, 0.0f, (m_particles[i].accumulatedTime - dissappearFactor) / (m_particles[i].lifetime - dissappearFactor));
		}
		else
			m_particles[i].rgba.w = 1.0f;
	}

	DestroyParticles();
}

void Engine::Emitter::DestroyParticles()
{
	uint32_t diedParticle = 0;
	uint32_t diedAmount = 0;

	for (size_t i = 0; i < m_particleCount; i++)
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

void Engine::Emitter::adjustParameters(const vec3& maxSpeed, const vec2& size, float spawnRate, float lifetime)
{
	m_maxSpeed = maxSpeed;
	m_size = size;
	m_particleMaxLifeTime = lifetime;
	
	if (m_spawnRate != spawnRate)
	{
		m_spawnRate = spawnRate;
		m_particles.resize((size_t)ceil(m_spawnRate * m_particleMaxLifeTime));
	}
}

void Engine::Emitter::setColor(const vec3& color)
{
	m_particleColor = color;
}


Engine::ParticleSystem* Engine::ParticleSystem::m_instance;
std::mutex Engine::ParticleSystem::m_mutex;

Engine::ParticleSystem* Engine::ParticleSystem::Init()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	
	if (m_instance == nullptr)
	{
		m_instance = new ParticleSystem();
	}

	return m_instance;
}

void Engine::ParticleSystem::Deinit()
{
	delete m_instance;
	m_instance = nullptr;
}

void Engine::ParticleSystem::addSmokeEmitter(const Emitter& emitter)
{
	m_emmiters.push_back(emitter);

	m_vertexBufferData.resize(m_vertexBufferData.size() + emitter.getMaxSize());
	m_vertexBufferSortedData.resize(m_vertexBufferSortedData.size() + emitter.getMaxSize());
	m_indexedDistance.resize(m_indexedDistance.size() + emitter.getMaxSize());
	m_sortedIndexedDistance.resize(m_sortedIndexedDistance.size() + emitter.getMaxSize());
}

Engine::Emitter* Engine::ParticleSystem::getEmitterByTransformId(uint32_t id)
{
	for (size_t i = 0; i < m_emmiters.size(); i++)
	{
		if (m_emmiters[i].getBindedTransformId() == id)
			return &m_emmiters[i];
	}
	return nullptr;
}

void Engine::ParticleSystem::Update(float deltaTime)
{
	for (size_t i = 0; i < m_emmiters.size(); i++)
	{
		m_emmiters[i].Update(deltaTime);
	}
}

void Engine::ParticleSystem::UpdateBuffers(const vec3& cameraPosition)
{

	particlesAmount = 0;
	for (size_t i = 0; i < m_emmiters.size(); i++)
	{
		particlesAmount += m_emmiters[i].getSize();
	}
	
	if (particlesAmount == 0)
		return;

	ParticleInstance* dst;
	dst = m_vertexBufferData.data();

	int index = 0;

	for (size_t i = 0; i < m_emmiters.size(); i++)
	{
		auto& particles = m_emmiters[i].getParticles();
		for (size_t j = 0; j < m_emmiters[i].getSize(); j++)
		{
			float fractionLifeTime = particles[j].accumulatedTime / particles[j].lifetime;
			float frameTime = particles[j].lifetime / (float)textureFrameCount;
			dst[index].position = particles[j].position;
			dst[index].rgba = particles[j].rgba;
			dst[index].rotation = particles[j].rotation * particles[j].accumulatedTime;
			dst[index].size = particles[j].size * (1.0f + fractionLifeTime);
			dst[index].frameIndex = (uint32_t)(particles[j].accumulatedTime / frameTime);
			dst[index].frameFraction = particles[j].accumulatedTime - frameTime * dst[index].frameIndex;

			m_indexedDistance[index].distance = (dst[index].position - cameraPosition).length_squared();
			m_indexedDistance[index].index = index;

			++index;
		}
	}

	RadixSort11(m_indexedDistance.data(), m_sortedIndexedDistance.data(), particlesAmount);

	for (int i = particlesAmount - 1; i > -1; i--)
	{
		m_vertexBufferSortedData[particlesAmount - i - 1] = m_vertexBufferData[m_sortedIndexedDistance[i].index];
	}

	m_vertexBuffer.create(m_vertexBufferSortedData.data(), (UINT)particlesAmount);
}

void Engine::ParticleSystem::SetSmokeTextures(std::shared_ptr<Texture> RLU, std::shared_ptr<Texture> DBF, std::shared_ptr<Texture> EMVA)
{
	m_RLU = RLU;
	m_DBF = DBF;
	m_EMVA = EMVA;

	PartilcleAtlasInfo data = { textureRowCount, textureColumnCount };
	m_cbTextureData.updateBuffer(&data);
}

void Engine::ParticleSystem::Render()
{
	auto context = D3D::GetInstance()->GetContext();
	m_shader->BindShader();

	m_vertexBuffer.bind();
	m_indexBuffer.bind();

	m_cbTextureData.bind(7u, VS);

	m_EMVA->BindTexture(20);
	m_RLU->BindTexture(21);
	m_DBF->BindTexture(22);

	context->DrawIndexedInstanced(6u, particlesAmount, 0, 0, 0);
}

Engine::ParticleSystem::ParticleSystem()
{
	textureFrameCount = textureRowCount * textureColumnCount;
	unsigned int indicies[6] = { 0, 1, 2, 2, 1, 3 };
	m_indexBuffer.create(indicies, 6u);
	m_cbTextureData.create();

	D3D11_INPUT_ELEMENT_DESC particleInputLayout[] =
	{
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(ParticleInstance, rgba), D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ParticleInstance, position), D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "ROTATION", 0, DXGI_FORMAT_R32_FLOAT, 0, offsetof(ParticleInstance, rotation), D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(ParticleInstance, size), D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "FRAMEFRACTION", 0, DXGI_FORMAT_R32_FLOAT, 0, offsetof(ParticleInstance, frameFraction), D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "FRAMEINDEX", 0, DXGI_FORMAT_R32_UINT, 0, offsetof(ParticleInstance, frameIndex), D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	m_shader = ShaderManager::CompileAndCreateShader("ParticleShader", L"Shaders\\Particles\\VertexShader.hlsl", L"Shaders\\Particles\\PixelShader.hlsl", nullptr, nullptr);
	
	auto layout = ShaderManager::CreateInputLayout("ParticleLayout", m_shader->vertexBlob.Get(), particleInputLayout, sizeof(particleInputLayout) / sizeof(D3D11_INPUT_ELEMENT_DESC));
	m_shader->BindInputLyout(layout);
}
