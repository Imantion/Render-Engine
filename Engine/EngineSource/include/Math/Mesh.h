#pragma once
#include "Math/matrix.h"
#include "Math/math.h"
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
		

		int trianglesAmount() const
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
		BVH bvh;
		
		
	};

	struct primitive
	{
		primitive() : position(0.0f) {}
		primitive(const vec3& position) : position(position) {}

		void setPosition(const vec3& pos)
		{
			position = pos;

			transformeMatrix = transformMatrix(position, vec3(0, 0, 1), vec3(1, 0, 0), vec3(0, 1, 0));
			invTransformeMatrix = mat4::Inverse(transformeMatrix);
		}

		vec3 getPosition() { return position; }
		virtual const Mesh* getMesh() = 0;

		mat4 transformeMatrix;
		mat4 invTransformeMatrix;
		Material material;

	private:
		vec3 position;
	};
}

