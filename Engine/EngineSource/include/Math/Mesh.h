#pragma once
#include "Math/vec.h"
#include "Math/matrix.h"
#include "Math/triangle.h"
#include <vector>

namespace Engine
{
	class Mesh
	{
	public:
		static Mesh* UniteCube()
		{
			Mesh* cube = new Mesh;

			cube->vertex = { vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1), vec3(1, 1, 0), vec3(1, 0, 1), vec3(0, 1, 1), vec3(1, 1, 1) };


			cube->buffer = { vertexBuffer(0,1,2), vertexBuffer(4,2,1), vertexBuffer(3,6,5),vertexBuffer(7,5,6),vertexBuffer(1,5,4), vertexBuffer(7,4,5),vertexBuffer(0,2,3),
			vertexBuffer(6,3,2),vertexBuffer(2,4,6),vertexBuffer(7,6,4),vertexBuffer(0,3,1),vertexBuffer(5,1,3) };

			return cube;
		}

		const triangle& getTriangle(uint8_t index)
		{
			return triangle(vertex[buffer[index].a], vertex[buffer[index].b], vertex[buffer[index].c]);
		}

		int trianglesAmount()
		{
			return buffer.size();
		}
	private:
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
		primitive() : position(0.0f) {}
		primitive(const vec3& position) : position(position) {}
		mat4 transformeMatrix;
		mat4 invTransformeMatrix;
		vec3 position;

		static std::unique_ptr<Mesh> mesh;
	};
}

