#include "Graphics/DecalSystem.h"
#include "Math/matrix.h"
#include "Graphics/TextureManager.h"
#include "Graphics/MeshSystem.h"
#include "Graphics/TransformSystem.h"
#include <algorithm>


Engine::DecalSystem* Engine::DecalSystem::m_instance;
std::mutex Engine::DecalSystem::m_mutex;


void Engine::DecalSystem::UpdateBuffer()
{
    uint32_t instanceCount = 0;
    for (size_t i = 0; i < m_perTexture.size(); i++)
    {
        instanceCount += (uint32_t)m_perTexture[i].instances.size();
    }

    if (instanceCount == 0)
        return;

    m_instanceBuffer.create(instanceCount, D3D11_USAGE_DYNAMIC);

    D3D11_MAPPED_SUBRESOURCE mapping;
    m_instanceBuffer.map(mapping);
    Instances::DecalInstance* dst = static_cast<Instances::DecalInstance*>(mapping.pData);

    uint32_t copiedNum = 0;
    for (size_t i = 0; i < m_perTexture.size(); i++)
    {
        for (size_t j = 0; j < m_perTexture[i].instances.size(); j++)
        {
            auto& instance = m_perTexture[i].instances[j];
            vec3 position = (vec3&)*TransformSystem::Init()->GetModelTransforms(instance.objectTransformID)[0].modelToWold[3];
            mat4 newTransform = instance.transform;
            (vec3&)*newTransform[3] = position + instance.relPosition;
            dst[copiedNum++] = Instances::DecalInstance{ newTransform, mat4::Inverse(newTransform), instance.objectID,instance.usedTextures,instance.roughness,instance.metalness,instance.decalColor };
        }
    }

    m_instanceBuffer.unmap();
}

void Engine::DecalSystem::AddDecal(const mat4& decalToWorld, uint32_t objectID, const Materials::DecalMaterial& material, vec3 albedo,float roughness, float metalness)
{
    auto iterator = std::find(m_perTexture.begin(), m_perTexture.end(), material);
    
    int transformId = MeshSystem::Init()->getTransformIdByMeshInstanceId(objectID);
    if (transformId == -1)
        throw("Invalid objectID");

    DecalData instance = { decalToWorld, objectID, (uint32_t)transformId, vec3(0.0f), material.usedTextures, roughness, metalness, albedo};
    instance.relPosition = (vec3&)*instance.transform[3] - (vec3&)*TransformSystem::Init()->GetModelTransforms(instance.objectTransformID)[0].modelToWold[3];
    if (iterator == m_perTexture.end())
    {
        PerTexture perTexture;
        perTexture.material = material;
        perTexture.instances.push_back(instance);
        m_perTexture.push_back(perTexture);
    }
    else
    {
        (*iterator).instances.push_back(instance);
    }
}

void Engine::DecalSystem::Draw()
{
    if (m_indexBuffer.getSize() == 0)
        return;

    m_shader->BindShader();
    m_indexBuffer.bind();
    m_vertexBuffer.bind(0u);
    m_instanceBuffer.bind(1u);

    UINT renderedInstances = 0;
    for (size_t i = 0; i < m_perTexture.size(); i++)
    {
        m_perTexture[i].material.normal->BindTexture(21u);
        m_perTexture[i].material.albedo->BindTexture(22u);
        m_perTexture[i].material.roughness->BindTexture(23u);
        m_perTexture[i].material.metalness->BindTexture(24u);
        

        D3D::GetInstance()->GetContext()->DrawIndexedInstanced(36u, (UINT)m_perTexture[i].instances.size(), 0, 0, renderedInstances);
        renderedInstances += (UINT)m_perTexture[i].instances.size();
    }
}

void Engine::DecalSystem::SetShader(std::shared_ptr<shader> shader)
{
    m_shader = shader;
}

Engine::DecalSystem* Engine::DecalSystem::Init()
{
    std::lock_guard lock(m_mutex);

    if (m_instance == nullptr)
    {
        m_instance = new DecalSystem();
    }

    return m_instance;
}

void Engine::DecalSystem::Deinit()
{
    delete m_instance;
    m_instance = nullptr;
}

Engine::DecalSystem::DecalSystem()
{
    unsigned int indices[] =
    {
        // Front face (counterclockwise winding)
        0, 1, 2,    0, 2, 3,   // Triangle 1, Triangle 2

        // Back face (counterclockwise winding)
        4, 6, 5,    4, 7, 6,   // Triangle 3, Triangle 4

        // Left face (counterclockwise winding)
        4, 3, 7,    4, 0, 3,   // Triangle 5, Triangle 6

        // Right face (counterclockwise winding)
        1, 5, 6,    1, 6, 2,   // Triangle 7, Triangle 8

        // Top face (counterclockwise winding)
        4, 5, 1,    4, 1, 0,   // Triangle 9, Triangle 10

        // Bottom face (counterclockwise winding)
        3, 2, 6,    3, 6, 7    // Triangle 11, Triangle 12
    };

    vec3 vertices[] =
    {
        // Front face
        { -0.5f,  0.5f, -0.5f }, // Top-left (0)
        {  0.5f,  0.5f, -0.5f }, // Top-right (1)
        {  0.5f, -0.5f, -0.5f }, // Bottom-right (2)
        { -0.5f, -0.5f, -0.5f }, // Bottom-left (3)

        // Back face
        { -0.5f,  0.5f,  0.5f }, // Top-left (4)
        {  0.5f,  0.5f,  0.5f }, // Top-right (5)
        {  0.5f, -0.5f,  0.5f }, // Bottom-right (6)
        { -0.5f, -0.5f,  0.5f }, // Bottom-left (7)
    };

    m_indexBuffer.create(indices, 36u);
    m_vertexBuffer.create(vertices, 8u);
}
