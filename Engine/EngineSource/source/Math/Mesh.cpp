#include "Math/Mesh.h"
#include "Math/triangle.h"
#include "Math/math.h"

Engine::Mesh* Engine::Mesh::UniteCube()
{
	Mesh* cube = new Mesh;

	cube->vertex = { vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1), vec3(1, 1, 0), vec3(1, 0, 1), vec3(0, 1, 1), vec3(1, 1, 1) };


	cube->buffer = { vertexBuffer(0,1,2), vertexBuffer(4,2,1), vertexBuffer(3,6,5),vertexBuffer(7,5,6),vertexBuffer(1,5,4), vertexBuffer(7,4,5),vertexBuffer(0,2,3),
	vertexBuffer(6,3,2),vertexBuffer(2,4,6),vertexBuffer(7,6,4),vertexBuffer(0,3,1),vertexBuffer(5,1,3) };


	return cube;
}

const Engine::triangle Engine::Mesh::getTriangle(uint8_t index) const
{
	return triangle(vertex[buffer[index].a], vertex[buffer[index].b], vertex[buffer[index].c]);
}

bool Engine::Mesh::intersect(const ray& r, hitInfo& hInfo) const
{
	uint32_t intersectedTriIndex;
	bool intersected = false;
	hitInfo hit;
	hInfo.t = FLT_MAX;
	for (uint32_t i = 0; i < buffer.size(); ++i) {
		if (hitTriangle(getTriangle(i), r, hit) && hit.t < hInfo.t)
		{
			hInfo = hit;
			intersectedTriIndex = i;
			intersected = true;
		}
	}
	
	return intersected;
}

void Engine::Mesh::computeBounds(const vec3& normal, float& near, float& far) const
{
	for (size_t i = 0; i < vertex.size(); i++)
	{
		float d = dot(normal, vertex[i]);

		near = std::min(near, d);
		far = std::max(far, d);
	}
}