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
	class ModelManager;

	class Model
	{
	public:
		Model() = default;
		Model(Model&& other) noexcept
			:m_meshes(std::move(other.m_meshes)),
			m_ranges(std::move(other.m_ranges)),
			m_vertices(std::move(other.m_vertices)),
			box(std::move(other.box)),
			m_indices(std::move(other.m_indices)),
			name(std::move(other.name))
		{
			// No additional work needed since std::move takes care of moving resources
		}
		Model(Model& other) = default;
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
		friend class ModelManager;

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

		void initUnitSphere();
		void initUnitQuad();

		std::shared_ptr<Model> AddModel(std::string name, Model&& model);
		std::shared_ptr<Model> loadModel(std::string path, bool flipBT = false);
		std::shared_ptr<Model> GetModel(std::string name);

	protected:
		std::unordered_map<std::string, std::shared_ptr<Model>> models;
		std::vector<Model> DefaultModdels;

	private:
		static std::mutex mutex_;
		static ModelManager* pInstance;
	};
}
