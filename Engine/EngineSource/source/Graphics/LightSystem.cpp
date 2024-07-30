#include "Graphics/LightSystem.h"
#include "Graphics/TransformSystem.h"
#include "Graphics/TextureManager.h"
#include "Graphics/MeshSystem.h"

Engine::LightSystem* Engine::LightSystem::m_instance;
std::mutex Engine::LightSystem::m_mutex; 

Engine::LightSystem* Engine::LightSystem::Init()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_instance == nullptr)
    {
        m_instance = new LightSystem;
    }

    return m_instance;
}

void Engine::LightSystem::Deinit()
{
    delete m_instance;
    m_instance = nullptr;
}

void Engine::LightSystem::AddDirectionalLight(const DirectionalLight& other)
{
    if (m_directionalLights.size() > MAX_DIRECTIONAL_LIGHTS)
        throw "Too many directional lights";

    m_directionalLights.push_back(other);
}

void Engine::LightSystem::AddFlashLight(const SpotLight& spotLight, std::shared_ptr<Texture> texture, float nearClip, float farClip)
{
    m_flashLight.light = spotLight;
    m_flashLight.flashLightMask = texture;
    flProjectionData.aspectRatio = (float)texture->getTextureWidth() / (float)texture->getTextureHeight();
    flProjectionData.nearClip = nearClip;
    flProjectionData.farClip = farClip;

    if (m_flashLight.light.bindedObjectId == -1)
    {
        throw "Spotlight must be attached!";
    }

    m_flashLight.isAttached = true;
}

void Engine::LightSystem::BindLightTextures()
{
    m_flashLight.flashLightMask->BindTexture(1u);
}

void Engine::LightSystem::SetFlashLightAttachedState(bool attach)
{
    m_flashLight.isAttached = attach;
}

void Engine::LightSystem::AddDirectionalLight(const vec3& direction, const vec3& radiance, float solidAngle)
{
    DirectionalLight directLight(direction, radiance, solidAngle);
    AddDirectionalLight(directLight);
}


void Engine::LightSystem::AddPointLight(const PointLight& pointLight)
{
    if (m_pointLights.size() > MAX_POINT_LIGHTS)
        throw "Too many point lights";

    m_pointLights.push_back(pointLight);
}

uint32_t Engine::LightSystem::AddPointLight(const vec3& irradiance, float radius, float distance, const vec3& pos, std::shared_ptr<Model> model)
{
    PointLight pointLight(irradiance, radius, distance, vec3(0.0f));
    Engine::TransformSystem::transforms inst = {
        Engine::transformMatrix(pos, Engine::vec3(0.0f, 0.0f, 1.0f) * radius, Engine::vec3(1.0f, 0.0f, 0.0f) * radius, Engine::vec3(0.0f, 1.0f, 0.0f) * radius)};

    pointLight.bindedObjectId = Engine::MeshSystem::Init()->emmisiveGroup.addModel(model, Materials::EmmisiveMaterial{}, inst, Engine::MeshSystem::EmmisiveInstance{ pointLight.radiance });

    AddPointLight(pointLight);

    return pointLight.bindedObjectId;
}

void Engine::LightSystem::AddPointLight(const vec3& irradiance, float radius, float distance, const vec3& pos, int objectToBindId)
{
    PointLight pointLight(irradiance, radius, distance, pos);
    pointLight.bindedObjectId = objectToBindId;
    AddPointLight(pointLight);
}

uint32_t Engine::LightSystem::AddSpotLight(const vec3& irradiance, float radius, float distance, const vec3& pos, const vec3& direction, float cutoffAngle, std::shared_ptr<Model> model)
{
    SpotLight spotLight(irradiance, radius, distance, pos, direction, cutoffAngle);
    Engine::TransformSystem::transforms inst = {
    Engine::transformMatrix(pos, Engine::vec3(0.0f, 0.0f, 1.0f) * radius, Engine::vec3(1.0f, 0.0f, 0.0f) * radius, Engine::vec3(0.0f, 1.0f, 0.0f) * radius) };

    spotLight.bindedObjectId = Engine::MeshSystem::Init()->emmisiveGroup.addModel(model, Materials::EmmisiveMaterial{}, inst, Engine::MeshSystem::EmmisiveInstance{ spotLight.radiance });

    AddSpotLight(spotLight);

    return spotLight.bindedObjectId;
}

void Engine::LightSystem::AddSpotLight(const vec3& irradiance, float radius, float distance, const vec3& pos, const vec3& direction, float cutoffAngle, int objectToBindID)
{
    SpotLight spotLight(irradiance, radius, distance, pos, direction, cutoffAngle);
    spotLight.bindedObjectId = objectToBindID;
    AddSpotLight(spotLight);
}

void Engine::LightSystem::AddSpotLight(const SpotLight& spotLight)
{
    if (m_spotLights.size() > MAX_SPOT_LIGHTS)
        throw "Too many spot lights";

    m_spotLights.push_back(spotLight);
}

void Engine::LightSystem::AddAreaLight(const AreaLight& areaLight)
{
    m_areaLight.push_back(areaLight);
}

Engine::SpotLight& Engine::LightSystem::GetSpotLight(uint32_t index)
{
    return m_spotLights[index];
}

Engine::SpotLight* Engine::LightSystem::GetSpotLightByTransformId(uint32_t index)
{
    for (size_t i = 0; i < m_spotLights.size(); i++)
    {
        if (m_spotLights[i].bindedObjectId == index)
            return &m_spotLights[i];
    }
    return nullptr;
}

Engine::PointLight* Engine::LightSystem::GetPointLightByTransformId(uint32_t index)
{
    for (size_t i = 0; i < m_pointLights.size(); i++)
    {
        if (m_pointLights[i].bindedObjectId == index)
            return &m_pointLights[i];
    }
    return nullptr;
}

void Engine::LightSystem::GetPointLightsPositions(std::vector<vec3>& positions)
{
    positions.reserve(m_pointLights.size());
    auto TS = TransformSystem::Init();
    for (size_t i = 0; i < m_pointLights.size(); i++)
    {
        auto& transform = TS->GetModelTransforms(m_pointLights[i].bindedObjectId)[0].modelToWold;
        positions.push_back((vec3&)*transform[3] + m_pointLights[i].position);
    }
}

void Engine::LightSystem::GetPointLightsRadius(std::vector<float>& radiuses)
{
    radiuses.reserve(m_pointLights.size());
    for (size_t i = 0; i < m_pointLights.size(); i++)
    {
        radiuses.push_back(m_pointLights[i].radius);
    }
}

void Engine::LightSystem::UpdateLightsBuffer()
{

    LightsData bufferData;
    auto TS = TransformSystem::Init();

    bufferData.dlSize = (UINT)m_directionalLights.size();
    bufferData.spSize = (UINT)m_spotLights.size();
    bufferData.plSize = (UINT)m_pointLights.size();

    for (size_t i = 0; i < m_directionalLights.size(); i++)
    {
        bufferData.directionalLights[i] = m_directionalLights[i];
    }

    for (size_t i = 0; i < m_pointLights.size(); i++)
    {
        if (m_pointLights[i].bindedObjectId != -1)
        {
            auto& bindedTransform = TS->GetModelTransforms(m_pointLights[i].bindedObjectId)[0].modelToWold;
            bufferData.pointLights[i].position = m_pointLights[i].position + (vec3&)(*bindedTransform[3]);
        }
        else
        {
            bufferData.pointLights[i].position = m_pointLights[i].position;
        }
        bufferData.pointLights[i].radiance = m_pointLights[i].radiance;
        bufferData.pointLights[i].radius = m_pointLights[i].radius;
    }
    
    for (size_t i = 0; i < m_spotLights.size(); i++)
    {
        if (m_spotLights[i].bindedObjectId != -1)
        {
            auto& bindedTransform = TS->GetModelTransforms(m_spotLights[i].bindedObjectId)[0].modelToWold;
            bufferData.spotLights[i].position = m_spotLights[i].position + (vec3&)(*bindedTransform[3]);
            bufferData.spotLights[i].direction = vec4(m_spotLights[i].direction, 0.0f) * bindedTransform;
            bufferData.spotLights[i].direction = bufferData.spotLights[i].direction.normalized();
            bufferData.flashLightsViewProjection = mat4::Inverse(bindedTransform) * projectionMatrix(0.5f, 0.1f, 10.0f, 100, 100);;
        }
        else
        {
            bufferData.spotLights[i].position = m_spotLights[i].position;
            bufferData.spotLights[i].direction = m_spotLights[i].direction;
        }
        
        bufferData.spotLights[i].radiance = m_spotLights[i].radiance;
        bufferData.spotLights[i].cutoffAngle = cosf(m_spotLights[i].cutoffAngle);
        bufferData.spotLights[i].radius = m_spotLights[i].radius;
    }

    if (m_flashLight.light.bindedObjectId != -1)
    {
        if (m_flashLight.isAttached)
        {
            auto& bindedTransform = TS->GetModelTransforms(m_flashLight.light.bindedObjectId)[0].modelToWold;
            m_flashLight.worldPosition = m_flashLight.light.position + (vec3&)(*bindedTransform[3]);
            m_flashLight.worldDirection = (vec4(m_flashLight.light.direction, 0.0f) * bindedTransform).normalized();
            m_flashLight.flashLightsViewProjection = mat4::Inverse(bindedTransform) * projectionMatrix(m_flashLight.light.cutoffAngle * 2.0f, flProjectionData.nearClip,
                                                                                                       flProjectionData.farClip, flProjectionData.aspectRatio);
        }
           
        bufferData.flashLight.direction = m_flashLight.worldDirection;
        bufferData.flashLight.position = m_flashLight.worldPosition;
        bufferData.flashLight.radiance = m_flashLight.light.radiance;
        bufferData.flashLight.cutoffAngle = cosf(m_flashLight.light.cutoffAngle);
        bufferData.flashLight.radius = m_flashLight.light.radius;
        bufferData.flashLightsViewProjection = m_flashLight.flashLightsViewProjection;

    }

    bufferData.alSize = (UINT)m_areaLight.size();
    for (size_t i = 0; i < m_areaLight.size(); i++)
    {
        auto& transform = TS->GetModelTransforms(m_areaLight[i].bindedTransform)[0].modelToWold;
        bufferData.areaLights[i].radiance = m_areaLight[i].radiance;
        bufferData.areaLights[i].verticesAmount = m_areaLight[i].verticesAmount;
        bufferData.areaLights[i].indicesAmount = m_areaLight[i].indicesAmount;

        for (size_t j = 0; j < m_areaLight[i].verticesAmount; j++)
        {
            bufferData.areaLights[i].vertices[j] = m_areaLight[i].vertices[j] * transform;
        }
        
        for (size_t j = 0; j < m_areaLight[i].indicesAmount; j++)
        {
            bufferData.areaLights[i].edges[j] = m_areaLight[i].edges[j];
        }

        bufferData.areaLights[i].radiance = m_areaLight[i].radiance;
        bufferData.areaLights[i].intensity = m_areaLight[i].intensity;
        bufferData.areaLights[i].bindedTransform = m_areaLight[i].bindedTransform;
    }


    m_lighsBuffer.updateBuffer(&bufferData);
}

void Engine::LightSystem::BindLigtsBuffer(UINT slot, UINT typeOfShader)
{
    m_lighsBuffer.bind(slot, typeOfShader);
}

Engine::LightSystem::LightSystem()
{
    m_lighsBuffer.create();
}


