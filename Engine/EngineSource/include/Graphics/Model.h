#pragma once
#include "Graphics/Buffers.h"
#include "Graphics/Mesh.h"
#include "Math/Box.hpp"
#include <vector>
#include <unordered_map>
#include <memory>

namespace Engine
{
	template <typename I, typename M>
	class OpaqueInstances;
	struct ray;
	struct hitInfo;

	class Model
	{
	public:
		struct MeshRange
		{
			uint32_t vertexOffset; // offset in vertices
			uint32_t indexOffset; // offset in indices
			uint32_t vertexNum; // num of vertices
			uint32_t indexNum; // num of indices
		};

		bool intersect(const ray& r, hitInfo& info);

		friend class ModelManager;
		template <typename I, typename M>
		friend class OpaqueInstances;

	private:
		std::vector<Mesh> m_meshes;
		std::vector<MeshRange> m_ranges; // where each mesh data is stored in m_vertices
		VertexBuffer<Mesh::vertex> m_vertices; // stores vertices of all meshes of this Model
		Box box;
		IndexBuffer m_indices; // stores vertex indices of all meshes of this Model
		std::string name;
	};


	class ModelManager
	{
	public:
		static ModelManager* Init();
		static void Deinit();
		static ModelManager* GetInstance() { return pInstance; }

		void loadModel(std::string path);
		std::shared_ptr<Model> GetModel(std::string name);

	protected:
		std::unordered_map<std::string, std::shared_ptr<Model>> models;

	private:
		static std::mutex mutex_;
		static ModelManager* pInstance;
	};
}
