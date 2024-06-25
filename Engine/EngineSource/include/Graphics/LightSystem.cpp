#include "LightSystem.h"
#include "Graphics/TransformSystem.h"

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

void Engine::LightSystem::AddDirectionalLight(const vec3& direction, const vec3& color, float intensity)
{
    DirectionalLight directLight(direction, color, intensity);
    AddDirectionalLight(directLight);
}


void Engine::LightSystem::AddPointLight(const PointLight& other)
{
    if (m_pointLights.size() > MAX_POINT_LIGHTS)
        throw "Too many point lights";

    m_pointLights.push_back(other);
}

void Engine::LightSystem::AddPointLight(const vec3& col, const vec3& pos, float intens, int objectToBindId)
{
    PointLight pointLight(col, pos, intens);
    pointLight.bindedObjectId = objectToBindId;
    AddPointLight(pointLight);
}

void Engine::LightSystem::AddSpotLight(const SpotLight& spotLight)
{
    if (m_spotLights.size() > MAX_SPOT_LIGHTS)
        throw "Too many spot lights";

    m_spotLights.push_back(spotLight);
}

Engine::SpotLight& Engine::LightSystem::GetSpotLight(uint32_t index)
{
    return m_spotLights[index];
}

void Engine::LightSystem::UpdateLightsBuffer()
{

    LightsData bufferData;
    auto TS = TransformSystem::Init();

    bufferData.dlSize = m_directionalLights.size();
    bufferData.spSize = m_spotLights.size();
    bufferData.plSize = m_pointLights.size();

    for (size_t i = 0; i < m_directionalLights.size(); i++)
    {
        bufferData.directionalLights[i] = m_directionalLights[i];
    }

    for (size_t i = 0; i < m_pointLights.size(); i++)
    {
        if (m_pointLights[i].bindedObjectId != -1)
        {
            auto bindedTransform = TS->GetModelTransforms(m_pointLights[i].bindedObjectId)[0].modelToWold;
            bufferData.pointLights[i].position = m_pointLights[i].position + (vec3&)(*bindedTransform[3]);
        }
        else
        {
            bufferData.pointLights[i].position = m_pointLights[i].position;
        }
        bufferData.pointLights[i].color = m_pointLights[i].color;
        bufferData.pointLights[i].intensity = m_pointLights[i].intensity;
    }
    
    for (size_t i = 0; i < m_spotLights.size(); i++)
    {
        if (m_spotLights[i].bindedObjectId != -1)
        {
            auto bindedTransform = TS->GetModelTransforms(m_spotLights[i].bindedObjectId)[0].modelToWold;
            bufferData.spotLights[i].position = m_spotLights[i].position + (vec3&)(*bindedTransform[3]);
            bufferData.spotLights[i].direction = vec4(m_spotLights[i].direction, 0.0f) * bindedTransform;
            bufferData.spotLights[i].direction = bufferData.spotLights[i].direction.normalized();
            bufferData.spotLightsViewProjection = mat4::Inverse(bindedTransform) * projectionMatrix(0.5f, 0.1f, 10.0f, 100, 100);;
        }
        else
        {
            bufferData.spotLights[i].position = m_spotLights[i].position;
            bufferData.spotLights[i].direction = m_spotLights[i].direction;
        }
        
        bufferData.spotLights[i].color = m_spotLights[i].color;
        bufferData.spotLights[i].cutoffAngle = m_spotLights[i].cutoffAngle;
        bufferData.spotLights[i].intensity = m_spotLights[i].intensity;
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


void Engine::LightSystem::AddSpotLight(const vec3& col, const vec3& pos, const vec3& direction, float cutoffAngle, float intens, int objectToBindID)
{
    SpotLight spotLight(col, pos, direction, cutoffAngle, intens);
    spotLight.bindedObjectId = objectToBindID;
    AddSpotLight(spotLight);
}



