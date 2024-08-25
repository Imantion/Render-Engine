#include "DecalSystem.h"
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
            vec3 position = (vec3&)*TransformSystem::Init()->GetModelTransforms(m_perTexture[i].instances[j].objectTransformID)[0].modelToWold[3];
            mat4 newTransform = m_perTexture[i].instances[j].transform;
            (vec3&)*newTransform[3] = position + m_perTexture[i].instances[j].relPosition;
            dst[copiedNum++] = Instances::DecalInstance{ newTransform, mat4::Inverse(newTransform), m_perTexture[i].instances[j].objectID };
        }
    }

    m_instanceBuffer.unmap();
}

void Engine::DecalSystem::AddDecal(const Materials::DecalMaterial& material, const mat4& decalToWorld, uint32_t objectID)
{
    auto iterator = std::find(m_perTexture.begin(), m_perTexture.end(), material);
    
    int transformId = MeshSystem::Init()->getTransformIdByMeshInstanceId(objectID);
    if (transformId == -1)
        throw("Invalid objectID");

    DecalData instance = { decalToWorld, objectID, (uint32_t)transformId};
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
    m_instanceBuffer.bind(0u);

    UINT renderedInstances = 0;
    for (size_t i = 0; i < m_perTexture.size(); i++)
    {
        m_perTexture[i].material.albedo->BindTexture(10u);
        m_perTexture[i].material.normal->BindTexture(11u);

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
        0, 2, 1,    0, 3, 2,   // Triangle 1, Triangle 2

        // Back face (counterclockwise winding)
        4, 5, 6,    4, 6, 7,   // Triangle 3, Triangle 4

        // Left face (counterclockwise winding)
        4, 3, 0,    4, 7, 3,   // Triangle 5, Triangle 6

        // Right face (counterclockwise winding)
        1, 6, 5,    1, 2, 6,   // Triangle 7, Triangle 8

        // Top face (counterclockwise winding)
        4, 1, 5,    4, 0, 1,   // Triangle 9, Triangle 10

        // Bottom face (counterclockwise winding)
        3, 6, 2,    3, 7, 6    // Triangle 11, Triangle 12
    };


    m_indexBuffer.create(indices, 36u);
}
