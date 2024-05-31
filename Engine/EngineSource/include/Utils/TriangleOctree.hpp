#pragma once
#include <Math/Box.hpp>
#include "Math/hitable.h"
#include <cmath>
#include <vector>
#include <memory>

namespace Engine
{
	struct ray;

	struct Mesh;


	class TriangleOctree
	{
	public:
		const static int PREFFERED_TRIANGLE_COUNT;
		const static float MAX_STRETCHING_RATIO;

		void clear() { m_mesh = nullptr; }
		bool inited() const { return m_mesh != nullptr; }

		void initialize(const Mesh& mesh);

		bool intersect(const ray& ray, hitInfo& nearest) const;

	protected:
		const Mesh* m_mesh = nullptr;
		std::vector<uint32_t> m_triangles;

		Box m_box;
		Box m_initialBox;

		std::unique_ptr<std::array<TriangleOctree, 8>> m_children;

		void initialize(const Mesh& mesh, const Box& parentBox, const vec3& parentCenter, int octetIndex);

		bool addTriangle(uint32_t triangleIndex, const vec3& V1, const vec3& V2, const vec3& V3, const vec3& center);

		bool intersectInternal(const ray& ray, hitInfo& nearest) const;
	};
}