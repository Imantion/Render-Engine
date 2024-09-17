#pragma once
#include <memory>
#include "Math/math.h"

namespace Engine
{
	class Texture;
}

namespace Materials
{

	struct Material
	{
		Engine::vec3 shortWaveColor;
		float padding0;
		Engine::vec3 longWaveColor;
		float padding1;
		std::shared_ptr<Engine::Texture> texture;
		bool operator==(const Material& other) const
		{
			return shortWaveColor == other.shortWaveColor && longWaveColor == other.longWaveColor;
		}
	};

	enum textureType
	{
		ROUGHNESS = 1,
		METALNESS = 2,
		ALBEDO = 4,
		NORMAL = 8,
	};

	struct TextureMaterial
	{
		std::shared_ptr<Engine::Texture> albedoTexture;
		std::shared_ptr<Engine::Texture> roughnessTexture;
		std::shared_ptr<Engine::Texture> metalnessTexture;
		std::shared_ptr<Engine::Texture> normalTexture;

		int usedTextures =  ROUGHNESS | METALNESS;
		float roughness = 0.0f;
		float metalness = 0.0f;
		float padding;

		bool operator==(const TextureMaterial& other) const
		{
			if (usedTextures != other.usedTextures)
				return false;
			
			return albedoTexture.get() == other.albedoTexture.get() && roughnessTexture.get() == other.roughnessTexture.get() && 
				metalnessTexture.get() == other.metalnessTexture.get() && normalTexture.get() == other.normalTexture.get() && 
				roughness == other.roughness && metalness == other.metalness;
		}
	};

	struct DissolutionMaterial
	{
		TextureMaterial opaqueTextures;

		std::shared_ptr<Engine::Texture> noiseTexture;

		bool operator==(const DissolutionMaterial& other) const
		{
			return opaqueTextures == other.opaqueTextures && noiseTexture.get() == other.noiseTexture.get();
		}
	};

	struct EmmisiveMaterial
	{
		Engine::vec4 padding;
		bool operator==(const EmmisiveMaterial& other) const
		{
			return true;
		}
	};

	struct DecalMaterial
	{
		std::shared_ptr<Engine::Texture> albedo;
		std::shared_ptr<Engine::Texture> normal;
		std::shared_ptr<Engine::Texture> metalness;
		std::shared_ptr<Engine::Texture> roughness;
		uint32_t usedTextures;

		bool operator==(const DecalMaterial& other) const
		{
			return albedo == other.albedo && normal == other.normal && metalness == other.metalness && roughness == other.roughness;
		}
	};

	using OpaqueTextureMaterial = TextureMaterial;
	using HologramMaterial = Material;
	using NormVisMaterial = Material;
	using ShadowMaterial = EmmisiveMaterial;
}