#pragma once
#include "Math/matrix.h"
#include "Math/vec.h"
#include "Math/BVH.h"
#include "Render/Material.h"
#include <vector>
#include <memory>

namespace Engine
{
	struct triangle;
	struct ray;

	class Mesh
	{
	public:
		static Mesh* UniteCube();
		

		const triangle getTriangle(uint8_t index) const;
		

		int trianglesAmount()
		{
			return buffer.size();
		}

		bool intersect(const ray& r, hitInfo& hInfo) const;

		int getSize() const { return buffer.size(); }

		void computeBounds(const vec3& normal, float& near, float& far) const;

	public:
		struct vertexBuffer
		{
			vertexBuffer(uint8_t a, uint8_t b, uint8_t c) : a(a), b(b), c(c) {}

			uint8_t a, b, c;
		};

		std::vector<vertexBuffer> buffer;
		std::vector<vec3> vertex;
		
		
	};

	struct primitive
	{
		primitive() : position(0.0f), bvh(nullptr) {}
		primitive(const vec3& position) : position(position) {}
		mat4 transformeMatrix;
		mat4 invTransformeMatrix;
		vec3 position;
		Material material;

		static std::unique_ptr<Mesh> mesh;
		std::shared_ptr<BVH> bvh;
	};
}

