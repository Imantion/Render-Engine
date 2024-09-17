#pragma once
#include "Math/math.h"
#include "Materials.h"

namespace Engine
{
	template<typename I, typename M>
	class OpaqueInstances;
	class Texture;
	struct mat4;
	struct vec3;
}

namespace Instances
{

	struct Instance
	{
	};

	struct EmmisiveInstance
	{
		Engine::vec3 emmisiveColor;
	};

	struct PBRInstance
	{
		int isSelected = 0;
		int shouldOverWriteMaterial = 0;
		float roughness = 0.0f;
		float metalness = 0.0f;
	};

	struct DecalInstance
	{
		Engine::mat4 decalToWorld;
		Engine::mat4 worldToDecal;
		uint32_t bindedObject;
		uint32_t usedTextures = Materials::NORMAL;
		float roughness = 0.05f;
		float metalness = 0.05f;
		Engine::vec3 decalColor;
	};

	struct DissolutionInstance
	{
		DissolutionInstance(float duration) : animationDuration(duration)
		{}

		float animationDuration;

		friend class Engine::OpaqueInstances<DissolutionInstance, Materials::DissolutionMaterial>;
	private:
		float passedTime = 0.0f;
	};
}