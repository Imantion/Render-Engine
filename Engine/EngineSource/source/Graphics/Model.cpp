#include "Graphics/Model.h"
#include "Graphics/D3D.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "Math/hitable.h"
#include "Math/math.h"
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
	pInstance = nullptr;
}

std::shared_ptr<Engine::Model> Engine::ModelManager::AddModel(std::string name, Model&& model_)
{
	auto existingModel = models.find(name);
	if (existingModel != models.end())
	{
		std::wstring message = L"Model with name " + std::wstring(name.begin(), name.end()) + L" alredy exists";

		MessageBox(nullptr, message.c_str(), L"Warning", MB_OK);

		return existingModel->second;
	}

	models[name] = std::make_shared<Model>(std::move(model_));
	auto& model = *models[name].get();
	size_t vertexAmout = 0;
	size_t trianglesAmount = 0;
	for (size_t i = 0; i <  model.m_meshes.size(); i++)
	{
		vertexAmout += model.m_meshes[i].vertices.size();
	}
	for (size_t i = 0; i < model.m_meshes.size(); i++)
	{
		trianglesAmount += model.m_meshes[i].triangles.size();
	}


	std::vector<Engine::Mesh::vertex> vertecies; vertecies.reserve(vertexAmout);
	std::vector<Engine::Mesh::triangle> triangles; triangles.reserve(trianglesAmount);

	for (size_t i = 0; i < model.m_meshes.size(); i++)
	{
		for (size_t j = 0; j < model.m_meshes[i].vertices.size(); j++)
		{
			vertecies.push_back(model.m_meshes[i].vertices[j]);
		}
	}
	for (size_t i = 0; i < model.m_meshes.size(); i++)
	{
		for (size_t j = 0; j < model.m_meshes[i].triangles.size(); j++)
		{
			triangles.push_back(model.m_meshes[i].triangles[j]);
		}
	}

	auto& m = models[name];
	m->m_vertices.create(vertecies.data(), (UINT)(vertexAmout));
	m->m_indices.create((unsigned int*)triangles.data(), (UINT)(trianglesAmount) * 3u);
	return models[name];
}

std::shared_ptr<Engine::Model> Engine::ModelManager::loadModel(std::string path, bool flipBT, std::vector<uint32_t>* materialIndexes, bool useSkeletalMesh)
{
	if (models.find(path) != models.end())
		return models.find(path)->second;
	// Load aiScene

	float sign = 1.0f;
	if (flipBT)
		sign = -1.0f;

	uint32_t flags;
	if (useSkeletalMesh)
		 flags = uint32_t(aiProcess_Triangulate | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);
	else
	 flags = uint32_t(aiProcess_Triangulate | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace);
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

	vec3 min = vec3(std::numeric_limits<float>().max());
	vec3 max = vec3(-std::numeric_limits<float>().max());
	
	if(materialIndexes)
		materialIndexes->reserve(numMeshes);

	for (uint32_t i = 0; i < numMeshes; ++i)
	{
		auto& srcMesh = assimpScene->mMeshes[i];
		auto& dstMesh = model->m_meshes[i];
		auto& dstMeshRange = model->m_ranges[i];

		if(materialIndexes)
			materialIndexes->push_back(srcMesh->mMaterialIndex);

		dstMesh.name = srcMesh->mName.C_Str();
		dstMesh.box.min = reinterpret_cast<vec3&>(srcMesh->mAABB.mMin);
		dstMesh.box.max = reinterpret_cast<vec3&>(srcMesh->mAABB.mMax);

		min.x = min.x > dstMesh.box.min.x ? dstMesh.box.min.x : min.x;
		min.y = min.y > dstMesh.box.min.y ? dstMesh.box.min.y : min.y;
		min.z = min.z > dstMesh.box.min.z ? dstMesh.box.min.z : min.z;

		max.x = max.x < dstMesh.box.max.x ? dstMesh.box.max.x : max.x;
		max.y = max.y < dstMesh.box.max.y ? dstMesh.box.max.y : max.y;
		max.z = max.z < dstMesh.box.max.z ? dstMesh.box.max.z : max.z;

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
			vert.tangent = reinterpret_cast<Engine::vec3&>(srcMesh->mTangents[v]) * sign;
			vert.bitangent = reinterpret_cast<Engine::vec3&>(srcMesh->mBitangents[v]) * -1.f * sign; // Flip V
			SetVertexBoneDataToDefault(vert);

			verticies.emplace_back(vert);
		}

		for (uint32_t f = 0; f < srcMesh->mNumFaces; ++f)
		{
			const auto& face = srcMesh->mFaces[f];
			//DEV_ASSERT(face.mNumIndices == 3);
			dstMesh.triangles[f] = *reinterpret_cast<Mesh::triangle*>(face.mIndices);

			indicies.emplace_back(dstMesh.triangles[f]);
		}

		ExtractBoneWeightForVertices(model, verticies, srcMesh, assimpScene);

		dstMesh.updateOctree();
		vertexOffset += dstMeshRange.vertexNum;
		indexOffset += dstMeshRange.indexNum;
	}

	model->box.min = min;
	model->box.max = max;
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
	model->m_indices.create(reinterpret_cast<unsigned int*>(indicies.data()), (UINT)indicies.size() * 3u);

	return model;
}

std::shared_ptr<Engine::Model> Engine::ModelManager::GetModel(std::string name)
{
	auto it = models.find(name);

	if (it == models.end())
		return nullptr;

	return (*it).second;
}

void Engine::ModelManager::SetVertexBoneDataToDefault(Mesh::vertex& vertex)
{
	for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		vertex.m_BoneIDs[i] = -1;
		vertex.m_Weights[i] = 0.0f;
	}
}

void Engine::ModelManager::SetVertexBoneData(Mesh::vertex& vertex, int boneID, float weight)
{
	for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
	{
		if (vertex.m_BoneIDs[i] < 0)
		{
			vertex.m_Weights[i] = weight;
			vertex.m_BoneIDs[i] = boneID;
			break;
		}
	}
}

void Engine::ModelManager::ExtractBoneWeightForVertices(std::shared_ptr<Model> model, std::vector<Mesh::vertex>& vertices, void* aimesh, const void* aiscene)
{
	aiMesh* mesh = reinterpret_cast<aiMesh*>(aimesh);
	const aiScene* scene = reinterpret_cast<const aiScene*>(aiscene);

	for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
	{
		int boneID = -1;
		std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
		if (model->m_BoneInfoMap.find(boneName) == model->m_BoneInfoMap.end())
		{
			BoneInfo newBoneInfo;
			newBoneInfo.id = model->m_BoneCounter;
			newBoneInfo.offset = reinterpret_cast<const mat4&>(mesh->mBones[boneIndex]->mOffsetMatrix.Transpose());
			model->m_BoneInfoMap[boneName] = newBoneInfo;
			boneID = model->m_BoneCounter;
			model->m_BoneCounter++;
		}
		else
		{
			boneID = model->m_BoneInfoMap[boneName].id;
		}
		assert(boneID != -1);
		auto weights = mesh->mBones[boneIndex]->mWeights;
		int numWeights = mesh->mBones[boneIndex]->mNumWeights;

		for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
		{
			int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;
			
			SetVertexBoneData(vertices[vertexId], boneID, weight);
		}
	}
}

bool Engine::Model::intersect(const ray& r, hitInfo& info)
{
	bool intersected = false;
	for (auto& mesh : m_meshes)
	{
		if(mesh.intersect(r, info))
			intersected = true;
	}

	return intersected;
}

void Engine::ModelManager::initUnitSphere()
{
	const uint32_t SIDES = 6;
	const uint32_t GRID_SIZE = 12;
	const uint32_t TRIS_PER_SIDE = GRID_SIZE * GRID_SIZE * 2;
	const uint32_t VERT_PER_SIZE = (GRID_SIZE + 1) * (GRID_SIZE + 1);

	Engine::Model model;
	model.m_ranges.resize(1);
	model.m_ranges[0].indexOffset = 0;
	model.m_ranges[0].vertexOffset = 0;
	model.m_ranges[0].vertexNum = VERT_PER_SIZE * SIDES;
	model.m_ranges[0].indexNum = TRIS_PER_SIDE * SIDES * 3u;

	model.name = "UNIT_SPHERE";
	model.box = Engine::Box::unit();
	model.m_meshes.emplace_back();
	Engine::Mesh& mesh = model.m_meshes.back();
	mesh.name = "UNIT_SPHERE";
	mesh.box = model.box;
	mesh.instances = { Engine::mat4::Identity() };
	mesh.invInstances = { Engine::mat4::Identity() };

	mesh.vertices.resize(VERT_PER_SIZE * SIDES);
	Mesh::vertex* vertex = mesh.vertices.data();

	int sideMasks[6][3] =
	{
		{ 2, 1, 0 },
		{ 0, 1, 2 },
		{ 2, 1, 0 },
		{ 0, 1, 2 },
		{ 0, 2, 1 },
		{ 0, 2, 1 }
	};

	float sideSigns[6][3] =
	{
		{ +1, +1, +1 },
		{ -1, +1, +1 },
		{ -1, +1, -1 },
		{ +1, +1, -1 },
		{ +1, -1, -1 },
		{ +1, +1, +1 }
	};

	for (int side = 0; side < SIDES; ++side)
	{
		for (int row = 0; row < GRID_SIZE + 1; ++row)
		{
			for (int col = 0; col < GRID_SIZE + 1; ++col)
			{
				Engine::vec3 v;
				v.x = col / float(GRID_SIZE) * 2.f - 1.f;
				v.y = row / float(GRID_SIZE) * 2.f - 1.f;
				v.z = 1.f;

				vertex[0] = Engine::Mesh::vertex::initial();

				vertex[0].pos[sideMasks[side][0]] = v.x * sideSigns[side][0];
				vertex[0].pos[sideMasks[side][1]] = v.y * sideSigns[side][1];
				vertex[0].pos[sideMasks[side][2]] = v.z * sideSigns[side][2];
				vertex[0].normal = vertex[0].pos = vertex[0].pos.normalized();

				vertex += 1;
			}
		}
	}

	mesh.triangles.resize(TRIS_PER_SIDE * SIDES);
	auto* triangle = mesh.triangles.data();

	for (int side = 0; side < SIDES; ++side)
	{
		uint32_t sideOffset = VERT_PER_SIZE * side;

		for (int row = 0; row < GRID_SIZE; ++row)
		{
			for (int col = 0; col < GRID_SIZE; ++col)
			{
				triangle[0].indices[0] = sideOffset + (row + 0) * (GRID_SIZE + 1) + col + 0;
				triangle[0].indices[1] = sideOffset + (row + 1) * (GRID_SIZE + 1) + col + 0;
				triangle[0].indices[2] = sideOffset + (row + 0) * (GRID_SIZE + 1) + col + 1;

				triangle[1].indices[0] = sideOffset + (row + 1) * (GRID_SIZE + 1) + col + 0;
				triangle[1].indices[1] = sideOffset + (row + 1) * (GRID_SIZE + 1) + col + 1;
				triangle[1].indices[2] = sideOffset + (row + 0) * (GRID_SIZE + 1) + col + 1;

				triangle += 2;
			}
		}
	}

	mesh.updateOctree();

	AddModel("UNIT_SPHERE", std::move(model));
}
void Engine::ModelManager::initUnitQuad()
{
	const uint32_t VERTS_PER_QUAD = 8; // 4 vertices for the front face + 4 for the back face
	const uint32_t TRIS_PER_QUAD = 4; // 2 triangles for the front face + 2 for the back face

	Engine::Model model;
	model.m_ranges.resize(1);
	model.m_ranges[0].indexOffset = 0;
	model.m_ranges[0].vertexOffset = 0;
	model.m_ranges[0].vertexNum = VERTS_PER_QUAD;
	model.m_ranges[0].indexNum = TRIS_PER_QUAD * 3;

	model.name = "UNIT_QUAD";
	model.box = Engine::Box::unit();
	model.m_meshes.emplace_back();
	Engine::Mesh& mesh = model.m_meshes.back();
	mesh.name = "UNIT_QUAD";
	mesh.box = model.box;
	mesh.instances = { Engine::mat4::Identity() };
	mesh.invInstances = { Engine::mat4::Identity() };

	mesh.vertices.resize(VERTS_PER_QUAD);
	Mesh::vertex* vertex = mesh.vertices.data();

	// Define the vertices of the quad
	// Front face
	vertex[0] = Engine::Mesh::vertex::initial();
	vertex[0].pos = Engine::vec3(-1.0f, -1.0f, 0.0f);
	vertex[0].normal = Engine::vec3(0.0f, 0.0f, 1.0f);

	vertex[1] = Engine::Mesh::vertex::initial();
	vertex[1].pos = Engine::vec3(1.0f, -1.0f, 0.0f);
	vertex[1].normal = Engine::vec3(0.0f, 0.0f, 1.0f);

	vertex[2] = Engine::Mesh::vertex::initial();
	vertex[2].pos = Engine::vec3(1.0f, 1.0f, 0.0f);
	vertex[2].normal = Engine::vec3(0.0f, 0.0f, 1.0f);

	vertex[3] = Engine::Mesh::vertex::initial();
	vertex[3].pos = Engine::vec3(-1.0f, 1.0f, 0.0f);
	vertex[3].normal = Engine::vec3(0.0f, 0.0f, 1.0f);

	// Back face
	vertex[4] = Engine::Mesh::vertex::initial();
	vertex[4].pos = Engine::vec3(-1.0f, -1.0f, 0.0f);
	vertex[4].normal = Engine::vec3(0.0f, 0.0f, -1.0f);

	vertex[5] = Engine::Mesh::vertex::initial();
	vertex[5].pos = Engine::vec3(1.0f, -1.0f, 0.0f);
	vertex[5].normal = Engine::vec3(0.0f, 0.0f, -1.0f);

	vertex[6] = Engine::Mesh::vertex::initial();
	vertex[6].pos = Engine::vec3(1.0f, 1.0f, 0.0f);
	vertex[6].normal = Engine::vec3(0.0f, 0.0f, -1.0f);

	vertex[7] = Engine::Mesh::vertex::initial();
	vertex[7].pos = Engine::vec3(-1.0f, 1.0f, 0.0f);
	vertex[7].normal = Engine::vec3(0.0f, 0.0f, -1.0f);

	// Define the indices for the triangles that make up the quad
	mesh.triangles.resize(TRIS_PER_QUAD);
	auto* triangle = mesh.triangles.data();

	// Front face triangles
	triangle[0].indices[0] = 0;
	triangle[0].indices[1] = 1;
	triangle[0].indices[2] = 2;

	triangle[1].indices[0] = 2;
	triangle[1].indices[1] = 3;
	triangle[1].indices[2] = 0;

	// Back face triangles
	triangle[2].indices[0] = 4;
	triangle[2].indices[1] = 6;
	triangle[2].indices[2] = 5;

	triangle[3].indices[0] = 6;
	triangle[3].indices[1] = 4;
	triangle[3].indices[2] = 7;

	mesh.updateOctree();

	AddModel("UNIT_QUAD", std::move(model));
}

