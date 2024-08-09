#pragma once
#include <vector>
#include <memory>
#include "Math/matrix.h"
#include "Graphics/Materials.h"
#include "Math/hitable.h"
#include "Graphics/Buffers.h"
#include "Graphics/Model.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/TextureManager.h"
#include "Graphics/TransformSystem.h"
#include "Graphics/Instances.h"

namespace Engine
{
	template <>
	class OpaqueInstances<Instances::DissolutionInstance,Materials::DissolutionMaterial>
	
}