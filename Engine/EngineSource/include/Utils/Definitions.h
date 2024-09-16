#pragma once

namespace Engine
{
	struct IndexedDistance
	{
		float distance;
		uint32_t index;
	};

	enum shaderTypes
	{
		VS = 1,
		HS = 2,
		DS = 4,
		GS = 8,
		PS = 16
	};
}
