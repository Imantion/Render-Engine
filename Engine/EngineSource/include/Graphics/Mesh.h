#pragma once
#include <array>
#include <vector>
#include <string>
#include "Math/vec.h"
#include "Math/matrix.h"
#include "Math/Box.hpp"
#include "Utils/TriangleOctree.hpp"

namespace Engine
{
	struct Mesh
	{
	public:
		

		struct vertex
		{
			vec3 pos;
			vec3 normal;
			vec3 tangent;
			vec3 bitangent;
			vec2 tc;
		};

		struct triangle
		{
			std::array<uint32_t, 3> indices;
		};

		Mesh() {}
		Mesh(const std::vector<vertex>& vert, const std::vector<triangle>& trian)
			: vertices(vert), triangles(trian)
		{
			box.reset();
			for (size_t i = 0; i < vertices.size(); i++)
			{
				box.expand(vertices[i].pos);
			}
			octree.initialize(*this);
		}

		Mesh(const Mesh& other) :
			vertices(other.vertices), triangles(other.triangles), box(other.box)
		{
			octree.initialize(*this);
		}

		bool intersect(const ray& r, hitInfo& info)
		{
			return octree.intersect(r, info);
		}

		void updateOctree()
		{
			octree.initialize(*this);
		}

	public:

		std::vector<mat4> instances;
		std::vector<mat4>  invInstances;
		Box box;
		std::vector<vertex> vertices;
		std::vector<triangle> triangles;
		std::string name;

	protected:
		TriangleOctree octree;
	};
}
