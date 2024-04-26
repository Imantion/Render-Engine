#pragma once
#include "Math/vec.h"

namespace Engine
{
	struct triangle
	{
		triangle(const triangle& t) : A(t.A), B(t.B), C(t.C) {}
		triangle(vec3 a, vec3 b, vec3 c) : A(a), B(b), C(c) {}
		vec3 A, B, C;
	};
}
