#pragma once

#define MAX_BONE_INFLUENCE 3

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
		PS = 16,
		CS = 32
	};

	enum UAVState
	{
		READ = 1,
		WRITE = 2,	
	};


}
