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



	struct TextureMaterial
	{
		std::shared_ptr<Engine::Texture> albedo;
		std::shared_ptr<Engine::Texture> roughness;
		std::shared_ptr<Engine::Texture> metalness;
		std::shared_ptr<Engine::Texture> normal;


		bool operator==(const TextureMaterial& other) const
		{
			return albedo.get() == other.albedo.get() && roughness.get() == other.roughness.get() && metalness.get() == other.metalness.get() && normal.get() == other.normal.get();
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

	using OpaqueTextureMaterial = TextureMaterial;
	using HologramMaterial = Material;
	using NormVisMaterial = Material;
}