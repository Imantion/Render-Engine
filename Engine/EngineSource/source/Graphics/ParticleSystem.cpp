#include "Graphics/ParticleSystem.h"
#include "Graphics/TransformSystem.h"
#include "Graphics/TextureManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Model.h"
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

			m_particles[i].speed.x = get_random(g_distribution_neg1_1);
			m_particles[i].speed.y = 1;
			m_particles[i].speed.z = get_random(g_distribution_neg1_1);
			m_particles[i].speed = m_particles[i].speed.normalized() * m_maxSpeed;

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

void Engine::ParticleSystem::InitGPUParticles()
{
	m_sphereModel = ModelManager::GetInstance()->GetModel("UNIT_SPHERE");

	RingBuffer::DataRange dataRange = { 0 , 0, 0 };
	RingBuffer::IndirectCall drawCall = { m_sphereModel->m_indices.getSize(), 0u, 0u, 0u, 0u, 6u, 0u, 0u, 0u, 0u, 0u, 1u, 1u};


	m_ringBuffer.m_dataRangebuffer.create(&dataRange, 1u, DXGI_FORMAT_R32_UINT, 0u, true);
	m_ringBuffer.m_indirectDrawbuffer.create(&drawCall, 1u, DXGI_FORMAT_R32_UINT, D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS, true);

	m_incinerationBuffer.create(10000u, true);
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

void Engine::ParticleSystem::UpdateGPUParticles()
{
	auto context = D3D::GetInstance()->GetContext();

	m_incinerationBuffer.bindWrite(0u);
	m_ringBuffer.m_dataRangebuffer.bindWrite(1u);
	m_ringBuffer.m_indirectDrawbuffer.bindWrite(2u);
	
	m_GPUParticlesUpdateCS->BindComputeShader();
	context->DispatchIndirect(m_ringBuffer.m_indirectDrawbuffer.getBuffer(), 10 * sizeof(UINT));


	m_GPUParticlesDrawCallUpdateCS->BindComputeShader();
	context->Dispatch(1, 1, 1);


	m_incinerationBuffer.unbindWrite(0u);
	m_ringBuffer.m_dataRangebuffer.unbindWrite(1u);
}

void Engine::ParticleSystem::SetSmokeTextures(std::shared_ptr<Texture> RLU, std::shared_ptr<Texture> DBF, std::shared_ptr<Texture> EMVA)
{
	m_RLU = RLU;
	m_DBF = DBF;
	m_EMVA = EMVA;

	PartilcleAtlasInfo data = { textureRowCount, textureColumnCount };
	m_cbTextureData.updateBuffer(&data);
}

void Engine::ParticleSystem::SetSparkTexture(std::shared_ptr<Texture> spark)
{
	m_spark = spark;
}

void Engine::ParticleSystem::SetGPUParticlesShaders(std::shared_ptr<shader> GPUBillboardshader, std::shared_ptr<shader> GPULightParticles, std::shared_ptr<shader> GPUParticlesUpdateCS, std::shared_ptr<shader> GPUParticlesDrawCallUpdateCS)
{
	m_GPUBillboardshader = GPUBillboardshader;
	m_GPULightParticles = GPULightParticles;
	m_GPUParticlesUpdateCS = GPUParticlesUpdateCS;
	m_GPUParticlesDrawCallUpdateCS = GPUParticlesDrawCallUpdateCS;
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

void Engine::ParticleSystem::RenderGPUParticles()
{
	auto context = D3D::GetInstance()->GetContext();

	m_GPULightParticles->BindShader();

	m_incinerationBuffer.bind(4u, VS);
	m_ringBuffer.m_dataRangebuffer.bind(5u, VS);

	m_sphereModel->m_vertices.bind();
	m_sphereModel->m_indices.bind();

	
	context->DrawIndexedInstancedIndirect(m_ringBuffer.m_indirectDrawbuffer.getBuffer(), 0u);

	m_incinerationBuffer.unbind(4u, VS);
	m_ringBuffer.m_dataRangebuffer.unbind(5u, VS);
}

void Engine::ParticleSystem::RenderGPUParticlesBillBoard()
{
	auto context = D3D::GetInstance()->GetContext();

	m_GPUBillboardshader->BindShader();

	m_incinerationBuffer.bind(4u, VS);
	m_ringBuffer.m_dataRangebuffer.bind(5u, VS);
	m_spark->BindTexture(4u, PS);

	m_indexBuffer.bind();


	context->DrawIndexedInstancedIndirect(m_ringBuffer.m_indirectDrawbuffer.getBuffer(), 5u * sizeof(UINT));

	m_incinerationBuffer.unbind(4u, VS);
	m_ringBuffer.m_dataRangebuffer.unbind(5u, VS);

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
