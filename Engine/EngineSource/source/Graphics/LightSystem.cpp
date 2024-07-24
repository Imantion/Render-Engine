#include "Graphics/LightSystem.h"
#include "Graphics/TransformSystem.h"
#include "Graphics/TextureManager.h"

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

void Engine::LightSystem::AddFlashLight(const SpotLight& spotLight, std::shared_ptr<Texture> texture, float aspectRatio, float nearClip, float farClip)
{
    m_flashLight.light = spotLight;
    m_flashLight.flashLightMask = texture;
    flProjectionData.aspectRatio = aspectRatio;
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
        bufferData.pointLights[i].color = m_pointLights[i].color;
        bufferData.pointLights[i].intensity = m_pointLights[i].intensity;
    }
    
    for (size_t i = 0; i < m_spotLights.size(); i++)
    {
        if (m_spotLights[i].bindedObjectId != -1)
        {
            auto& bindedTransform = TS->GetModelTransforms(m_spotLights[i].bindedObjectId)[0].modelToWold;
            bufferData.spotLights[i].position = m_spotLights[i].position + (vec3&)(*bindedTransform[3]);
            bufferData.spotLights[i].direction = vec4(m_spotLights[i].direction, 0.0f) * bindedTransform;
            bufferData.spotLights[i].direction = bufferData.spotLights[i].direction.normalized();
        }
        else
        {
            bufferData.spotLights[i].position = m_spotLights[i].position;
            bufferData.spotLights[i].direction = m_spotLights[i].direction;
        }
        
        bufferData.spotLights[i].color = m_spotLights[i].color;
        bufferData.spotLights[i].cutoffAngle = cosf(m_spotLights[i].cutoffAngle);
        bufferData.spotLights[i].intensity = m_spotLights[i].intensity;
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
        bufferData.flashLight.color = m_flashLight.light.color;
        bufferData.flashLight.cutoffAngle = cosf(m_flashLight.light.cutoffAngle);
        bufferData.flashLight.intensity = m_flashLight.light.intensity;
        bufferData.flashLightsViewProjection = m_flashLight.flashLightsViewProjection;

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



