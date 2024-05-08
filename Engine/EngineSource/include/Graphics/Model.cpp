#include "Graphics/Model.h"
#include "Graphics/D3D.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include <functional>

std::mutex Engine::ModelManager::mutex_;
Engine::ModelManager* Engine::ModelManager::pInstance = nullptr;

Engine::ModelManager* Engine::ModelManager::Init()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (!pInstance)
	{
		pInstance = new ModelManager();
	}

	return pInstance;
}

void Engine::ModelManager::Deinit()
{
	delete pInstance;
}

void Engine::ModelManager::loadModel(std::string path)
{
	if (models.find(path) != models.end())
		return;
	// Load aiScene

	uint32_t flags = uint32_t(aiProcess_Triangulate | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace);
	// aiProcess_Triangulate - ensure that all faces are triangles and not polygonals, otherwise triangulare them
	// aiProcess_GenBoundingBoxes - automatically compute bounding box, though we could do that manually
	// aiProcess_ConvertToLeftHanded - Assimp assumes left-handed basis orientation by default, convert for Direct3D
	// aiProcess_CalcTangentSpace - computes tangents and bitangents, they are used in advanced lighting

	Assimp::Importer importer;
	const aiScene* assimpScene = importer.ReadFile(path, flags);
	/*DEV_ASSERT(assimpScene);*/

	uint32_t numMeshes = assimpScene->mNumMeshes;

	// Load vertex data

	std::shared_ptr<Model> model = std::make_shared<Model>(Model());
	models[path] = model;
	model->name = path;
	model->box = {};
	model->m_meshes.resize(numMeshes);
	model->m_ranges.resize(numMeshes);

	static_assert(sizeof(vec3) == sizeof(aiVector3D), "vec3 doesn't equel to aiVecotr3D");
	static_assert(sizeof(Mesh::triangle) == 3 * sizeof(uint32_t), "Triangle is not 3 * uint32_T");

	int vertexOffset = 0, indexOffset = 0;

	int vertexAmount = 0, facesAmount = 0;
	for (size_t i = 0; i < numMeshes; i++)
	{
		vertexAmount += assimpScene->mMeshes[i]->mNumVertices;
		facesAmount += assimpScene->mMeshes[i]->mNumFaces;
	}

	std::vector<Mesh::vertex> verticies; verticies.reserve(vertexAmount);
	std::vector<Mesh::triangle> indicies; indicies.reserve(facesAmount);
	
	for (uint32_t i = 0; i < numMeshes; ++i)
	{
		auto& srcMesh = assimpScene->mMeshes[i];
		auto& dstMesh = model->m_meshes[i];
		auto& dstMeshRange = model->m_ranges[i];

		dstMesh.name = srcMesh->mName.C_Str();
		dstMesh.box.min = reinterpret_cast<vec3&>(srcMesh->mAABB.mMin);
		dstMesh.box.max = reinterpret_cast<vec3&>(srcMesh->mAABB.mMax);

		dstMesh.vertices.resize(srcMesh->mNumVertices);
		dstMesh.triangles.resize(srcMesh->mNumFaces);
		dstMeshRange.vertexNum = srcMesh->mNumVertices;
		dstMeshRange.vertexOffset = vertexOffset;
		dstMeshRange.indexNum = srcMesh->mNumFaces * 3;
		dstMeshRange.indexOffset = indexOffset;

		for (uint32_t v = 0; v < srcMesh->mNumVertices; ++v)
		{
			Mesh::vertex& vert = dstMesh.vertices[v];
			vert.pos = reinterpret_cast<Engine::vec3&>(srcMesh->mVertices[v]);
			vert.tc = reinterpret_cast<Engine::vec2&>(srcMesh->mTextureCoords[0][v]);
			vert.normal = reinterpret_cast<Engine::vec3&>(srcMesh->mNormals[v]);
			vert.tangent = reinterpret_cast<Engine::vec3&>(srcMesh->mTangents[v]);
			vert.bitangent = reinterpret_cast<Engine::vec3&>(srcMesh->mBitangents[v]) * -1.f; // Flip V

			verticies.push_back(vert);
		}

		for (uint32_t f = 0; f < srcMesh->mNumFaces; ++f)
		{
			const auto& face = srcMesh->mFaces[f];
			//DEV_ASSERT(face.mNumIndices == 3);
			dstMesh.triangles[f] = *reinterpret_cast<Mesh::triangle*>(face.mIndices);

			indicies.push_back(dstMesh.triangles[f]);
		}

		dstMesh.updateOctree();
		vertexOffset += dstMeshRange.vertexNum;
		indexOffset += dstMeshRange.indexNum;
	}

	// Recursively load mesh instances (meshToModel transformation matrices)

	std::function<void(aiNode*)> loadInstances;
	loadInstances = [&loadInstances, &model](aiNode* node)
		{
			const mat4 nodeToParent = reinterpret_cast<const mat4&>(node->mTransformation.Transpose());
			const mat4 parentToNode = mat4::Inverse(nodeToParent);

			// The same node may contain multiple meshes in its space, referring to them by indices
			for (uint32_t i = 0; i < node->mNumMeshes; ++i)
			{
				uint32_t meshIndex = node->mMeshes[i];
				model->m_meshes[meshIndex].instances.push_back(nodeToParent);
				model->m_meshes[meshIndex].invInstances.push_back(parentToNode);
			}

			for (uint32_t i = 0; i < node->mNumChildren; ++i)
				loadInstances(node->mChildren[i]);
		};

	loadInstances(assimpScene->mRootNode);

	model->m_vertices.create(verticies.data(), (UINT)verticies.size());
	model->m_indices.create(reinterpret_cast<unsigned int*>(indicies.data()), indicies.size() * 3u);
}

std::shared_ptr<Engine::Model> Engine::ModelManager::GetModel(std::string name)
{
	auto it = models.find(name);

	if (it == models.end())
		return nullptr;

	return (*it).second;
}
