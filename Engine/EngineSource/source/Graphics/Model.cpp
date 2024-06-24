#include "Graphics/Model.h"
#include "Graphics/D3D.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
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
	if (models.find(name) != models.end())
		throw "Model with this name alredy exists!";

	models[name] = std::make_shared<Model>(std::move(model_));
	auto& model = *models[name].get();
	int vertexAmout = 0;
	int trianglesAmount = 0;
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

	auto m = models[name];
	m->m_vertices.create(vertecies.data(), (UINT)(vertexAmout));
	m->m_indices.create((unsigned int*)triangles.data(), (UINT)(trianglesAmount) * 3u);
	return models[name];
}

std::shared_ptr<Engine::Model> Engine::ModelManager::loadModel(std::string path)
{
	if (models.find(path) != models.end())
		return models.find(path)->second;
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

void Engine::ModelManager::initUnitSphereFlat()
{
	const uint32_t SIDES = 6;
	const uint32_t GRID_SIZE = 12;
	const uint32_t TRIS_PER_SIDE = GRID_SIZE * GRID_SIZE * 2;
	const uint32_t VERT_PER_SIZE = 3 * TRIS_PER_SIDE;

	Engine::Model model;
	model.name = "UNIT_SPHERE_FLAT";
	model.box = Engine::Box::empty();
	model.m_ranges.resize(1);
	model.m_ranges[0].indexOffset = 0;
	model.m_ranges[0].vertexOffset = 0;
	model.m_ranges[0].vertexNum = VERT_PER_SIZE * SIDES;
	model.m_ranges[0].indexNum = TRIS_PER_SIDE * SIDES * 3u;

	model.m_meshes.emplace_back();
	Engine::Mesh& mesh = model.m_meshes.back();
	mesh.name = "UNIT_SPHERE_FLAT";
	mesh.box = model.box;
	mesh.instances = { Engine::mat4::Identity() };
	mesh.invInstances = { Engine::mat4::Identity() };

	mesh.vertices.resize(VERT_PER_SIZE * SIDES);
	Engine::Mesh::vertex* vertex = mesh.vertices.data();

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
		for (int row = 0; row < GRID_SIZE; ++row)
		{
			for (int col = 0; col < GRID_SIZE; ++col)
			{
				float left = (col + 0) / float(GRID_SIZE) * 2.f - 1.f;
				float right = (col + 1) / float(GRID_SIZE) * 2.f - 1.f;
				float bottom = (row + 0) / float(GRID_SIZE) * 2.f - 1.f;
				float top = (row + 1) / float(GRID_SIZE) * 2.f - 1.f;

				Engine::vec3 quad[4] =
				{
					{ left, bottom, 1.f },
					{ left, top, 1.f },
					{ right, bottom, 1.f },
					{ right, top, 1.f }
				};

				vertex[0] = vertex[1] = vertex[2] = vertex[3] = Engine::Mesh::vertex::initial();

				auto setPos = [sideMasks, sideSigns](int side, Engine::Mesh::vertex& dst, const  Engine::vec3& pos)
					{
						dst.pos[sideMasks[side][0]] = pos.x * sideSigns[side][0];
						dst.pos[sideMasks[side][1]] = pos.y * sideSigns[side][1];
						dst.pos[sideMasks[side][2]] = pos.z * sideSigns[side][2];
						dst.pos = dst.pos.normalized();
					};

				setPos(side, vertex[0], quad[0]);
				setPos(side, vertex[1], quad[1]);
				setPos(side, vertex[2], quad[2]);

				{
					Engine::vec3 AB = vertex[1].pos - vertex[0].pos;
					Engine::vec3 AC = vertex[2].pos - vertex[0].pos;
					vertex[0].normal = vertex[1].normal = vertex[2].normal = Engine::cross(AB,AC).normalized();
				}

				vertex += 3;

				setPos(side, vertex[0], quad[1]);
				setPos(side, vertex[1], quad[3]);
				setPos(side, vertex[2], quad[2]);

				{
					Engine::vec3 AB = vertex[1].pos - vertex[0].pos;
					Engine::vec3 AC = vertex[2].pos - vertex[0].pos;
					vertex[0].normal = vertex[1].normal = vertex[2].normal = Engine::cross(AB,AC).normalized();
				}

				vertex += 3;
			}
		}
	}

	mesh.updateOctree();

	AddModel("UNIT_SPHERE_FLAT", std::move(model));
}
