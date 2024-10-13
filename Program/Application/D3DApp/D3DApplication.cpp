#include "D3DApplication.h"
#include "Graphics/Engine.h"
#include "Graphics/ShaderManager.h"
#include "Input/Input.h"
#include "Render/Camera.h"
#include "Math/hitable.h"
#include "Math/math.h"
#include "Graphics/Renderer.h"
#include "Math/quaternion.h"
#include "Graphics/MeshSystem.h"
#include "Graphics/TextureManager.h"
#include "Graphics/PostProcess.h"	
#include "Graphics/SkyBox.h"
#include "Graphics/LightSystem.h"
#include "Graphics/ParticleSystem.h"
#include "Graphics/DecalSystem.h"
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Graphics/ShadowSystem.h"
#include "Utils/ISelected.h"
#include "Utils/Random.h"
#include <assert.h>

#ifdef UNICODE
typedef std::wostringstream tstringstream;
#else
typedef std::ostringstream tstringstream;
#endif

Engine::vec2 previousMousePosition;
static std::vector<Materials::DissolutionMaterial> samuraiDisolutionMaterial;
static Materials::DissolutionMaterial cubeDisolutionMaterial;
static Materials::DecalMaterial decalMaterial;
static Engine::vec2 decalRoughMetal = Engine::vec2(0.05f,0.05f);
static Engine::vec3 decalAlbedo = Engine::vec3(0.0f,0.2f,1.0f);
static float animationDuration = 4.0f;
Engine::Emitter* pEmitter;

static enum objectToSpawn
{
	SAMURAI,
	CRATE
} modelToSpawn;

static float cameraSpeed = 2.0f;

static auto changepos = [](Engine::TransformSystem::transforms& inst, const Engine::vec3& pos) {
	for (size_t i = 0; i < 3; i++) {
		inst.modelToWold[3][i] = pos[i];
	}
	};

static auto changescale = [](Engine::TransformSystem::transforms& inst, int axis, const float scale) {
	for (size_t i = 0; i < 3; i++) {
		inst.modelToWold[axis][i] *= scale;
		
	}
	};

static void InitMeshSystem()
{
	D3D11_INPUT_ELEMENT_DESC ied[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TC", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TOWORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"ISSELECTED", 0, DXGI_FORMAT_R32_SINT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"SHOULDOVERWRITE", 0, DXGI_FORMAT_R32_SINT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"ROUGHNESS", 0, DXGI_FORMAT_R32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"METALNESS", 0, DXGI_FORMAT_R32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"OBJECTID", 0, DXGI_FORMAT_R32_UINT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};

	D3D11_INPUT_ELEMENT_DESC normalIED[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TC", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TOWORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"OBJECTID", 0, DXGI_FORMAT_R32_UINT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};

	D3D11_INPUT_ELEMENT_DESC secondIed[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TC", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TOWORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"EMISSION", 0, DXGI_FORMAT_R32G32B32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"OBJECTID", 0, DXGI_FORMAT_R32_UINT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};

	D3D11_INPUT_ELEMENT_DESC thirdIed[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TC", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TOWORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"DURATION", 0, DXGI_FORMAT_R32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TIMER", 0, DXGI_FORMAT_R32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"OBJECTID", 0, DXGI_FORMAT_R32_UINT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};

	D3D11_INPUT_ELEMENT_DESC decalIED[] = {
		{"VERTEX", 0, DXGI_FORMAT_R32G32B32_FLOAT , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TOWORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TOWORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TOWORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TOWORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TODECAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TODECAL", 1, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TODECAL", 2, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TODECAL", 3, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"OBJECTID",0, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"USEDTEXTURES",0, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"ROUGHNESS",0, DXGI_FORMAT_R32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"METALNESS",0, DXGI_FORMAT_R32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"ALBEDO",0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};

	D3D11_INPUT_ELEMENT_DESC incinerationIED[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TC", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TOWORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"TOWORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"SPHEREPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"PARTICLECOLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"SPHERERADIUS", 0, DXGI_FORMAT_R32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"SPHEREPREVRADIUS", 0, DXGI_FORMAT_R32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"OBJECTID", 0, DXGI_FORMAT_R32_UINT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};

	D3D11_INPUT_ELEMENT_DESC gpuSphereIED[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TC", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	auto decalShader = Engine::ShaderManager::CompileAndCreateShader("DecalShader", L"Shaders\\Decal\\DecalVS.hlsl", L"Shaders\\Decal\\DecalPS.hlsl", nullptr, nullptr);

	auto emissiveShader = Engine::ShaderManager::CompileAndCreateShader("EmmisiveShader", L"Shaders\\emissive\\emissiveVS.hlsl",
		L"Shaders\\emissive\\emissivePS.hlsl", nullptr, nullptr);

	auto DefferedEmissiveShader = Engine::ShaderManager::CompileAndCreateShader("DefferedEmissiveShader", L"Shaders\\fullScreenVS.hlsl",
		L"Shaders\\emissive\\DefferedEmissivePS.hlsl", nullptr, nullptr);

	auto GemissiveShader = Engine::ShaderManager::CompileAndCreateShader("GEmmisiveShader", L"Shaders\\emissive\\emissiveVS.hlsl",
		L"Shaders\\emissive\\EmissiveGBufferPS.hlsl", nullptr, nullptr);

	auto NormalVisColor = Engine::ShaderManager::CompileAndCreateShader("NormalVisColor", L"Shaders\\normalColor\\VertexShader.hlsl",
		L"Shaders\\normalColor\\PixelShader.hlsl", nullptr, nullptr);

	auto dissolutionShader = Engine::ShaderManager::CompileAndCreateShader("DissolutionShader", L"Shaders\\opaqueShader\\dissolutionVS.hlsl", L"Shaders\\opaqueShader\\dissolutionPS.hlsl",
		nullptr, nullptr);

	auto GdissolutionShader = Engine::ShaderManager::CompileAndCreateShader("GDissolutionShader", L"Shaders\\opaqueShader\\dissolutionVS.hlsl", L"Shaders\\opaqueShader\\DissolutionGBufferPS.hlsl",
		nullptr, nullptr);

	auto textureMap = Engine::ShaderManager::CompileAndCreateShader("texture", L"Shaders\\crateTextMap\\CrateVS.hlsl",
		L"Shaders\\crateTextMap\\CratePS.hlsl", nullptr, nullptr);

	auto incenerationShader = Engine::ShaderManager::CompileAndCreateShader("Incineration", L"Shaders\\opaqueShader\\IncinerationVS.hlsl",
		L"Shaders\\opaqueShader\\IncinerationPS.hlsl", L"Shaders\\opaqueShader\\IncinerationHS.hlsl", L"Shaders\\opaqueShader\\IncinerationDS.hlsl", L"Shaders\\opaqueShader\\IncinerationGS.hlsl", nullptr, nullptr,
		D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	auto gpuSphereShader = Engine::ShaderManager::CompileAndCreateShader("GPUSphere", L"Shaders\\GPUParticles\\SphereParticleVS.hlsl",
				L"Shaders\\GPUParticles\\SphereParticlePS.hlsl", nullptr, nullptr);

	D3D_SHADER_MACRO shaders[] = { "MAX_DIRECTIONAL_LIGHTS", "1",
		"MAX_POINT_LIGHTS", "10",
		"MAX_SPOT_LIGHTS","10",
		NULL,NULL};

	auto opaqueShader = Engine::ShaderManager::CompileAndCreateShader("opaque", L"Shaders\\opaqueShader\\opaqueVS.hlsl",
		L"Shaders\\opaqueShader\\opaquePS.hlsl", nullptr, shaders);

	auto GopaqueShader = Engine::ShaderManager::CompileAndCreateShader("Gopaque", L"Shaders\\opaqueShader\\opaqueVS.hlsl",
		L"Shaders\\opaqueShader\\OpaqueGBufferPS.hlsl", nullptr, shaders);

	auto DefferedOpaqueShader = Engine::ShaderManager::CompileAndCreateShader("deferredopaque", L"Shaders\\fullScreenVS.hlsl",
		L"Shaders\\opaqueShader\\DefferedOpaquePS.hlsl", nullptr, shaders);

	auto NormalVisLines = Engine::ShaderManager::CompileAndCreateShader("NormalVisLines", L"Shaders\\normalLines\\VertexShader.hlsl",
		L"Shaders\\normalLines\\PixelShader.hlsl", L"Shaders\\normalLines\\HullShader.hlsl", L"Shaders\\normalLines\\DomainShader.hlsl",
		L"Shaders\\normalLines\\GSnormal.hlsl", nullptr, nullptr, D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);


	auto shadowShader = Engine::ShaderManager::CompileAndCreateShader("PLshadow", L"Shaders\\Shadow\\PointLightShadowVS.hlsl", L"Shaders\\Shadow\\PointLightShadowPS.hlsl",
		nullptr, nullptr, L"Shaders\\Shadow\\PointLightShadowGS.hlsl", nullptr, nullptr);

	auto shadowShader2 = Engine::ShaderManager::CompileAndCreateShader("shadow", L"Shaders\\Shadow\\ShadowVS.hlsl", L"Shaders\\Shadow\\ShadowPS.hlsl", nullptr, nullptr);

	auto shadowShader3 = Engine::ShaderManager::CompileAndCreateShader("DissPLshadow", L"Shaders\\Shadow\\Dissolution\\PointLightShadowVS.hlsl", L"Shaders\\Shadow\\Dissolution\\PointLightShadowPS.hlsl",
		nullptr, nullptr, L"Shaders\\Shadow\\Dissolution\\PointLightShadowGS.hlsl", nullptr, nullptr);

	auto shadowShader4 = Engine::ShaderManager::CompileAndCreateShader("DissShadow", L"Shaders\\Shadow\\Dissolution\\ShadowVS.hlsl", L"Shaders\\Shadow\\Dissolution\\ShadowPS.hlsl", nullptr, nullptr);

	auto incerShadowSLDL = Engine::ShaderManager::CompileAndCreateShader("IncinerationShadow", L"Shaders\\Shadow\\Incineration\\ShadowVS.hlsl", L"Shaders\\Shadow\\Incineration\\ShadowPS.hlsl", nullptr, nullptr);
	auto incerShadowPL = Engine::ShaderManager::CompileAndCreateShader("IncinerationPLShadow", L"Shaders\\Shadow\\Incineration\\PointLightShadowVS.hlsl", L"Shaders\\Shadow\\Incineration\\PointLightShadowPS.hlsl",
		nullptr, nullptr, L"Shaders\\Shadow\\Incineration\\PointLightShadowGS.hlsl", nullptr, nullptr);
	
	auto GPUbillboardShader = Engine::ShaderManager::CompileAndCreateShader("GPUBillBoard", L"Shaders\\GPUParticles\\BillboardVS.hlsl",
		L"Shaders\\GPUParticles\\BillboardPS.hlsl", nullptr, nullptr);

	NormalVisLines->DisableShader();
	if (!NormalVisColor)
		throw std::runtime_error("Failed to compile and create shader!");

	auto HologramGroup = Engine::ShaderManager::CompileAndCreateShader("HologramGroup", L"Shaders\\Hologram\\HologramGBuffer.shader",
		L"Shaders\\Hologram\\HologramGBuffer.shader", nullptr, nullptr, L"Shaders\\Hologram\\GSHologram.hlsl",
		nullptr, nullptr, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, "vsMain", "psMain");

	auto cs1 = Engine::ShaderManager::CompileAndCreateComputeShader("cs1", L"Shaders\\GPUParticles\\ParticleUpdateCS.hlsl", nullptr);
	auto cs2 = Engine::ShaderManager::CompileAndCreateComputeShader("cs2", L"Shaders\\GPUParticles\\ParticleIndirectDataUpdateCS.hlsl", nullptr);

	if (!HologramGroup)
		throw std::runtime_error("Failed to compile and create shader!");

	auto decalLayout = Engine::ShaderManager::CreateInputLayout("Decal", decalShader->vertexBlob.Get(), decalIED, sizeof(decalIED) / sizeof(D3D11_INPUT_ELEMENT_DESC));
	auto inputLayout = Engine::ShaderManager::CreateInputLayout("Default", opaqueShader->vertexBlob.Get(), ied, sizeof(ied) / sizeof(D3D11_INPUT_ELEMENT_DESC));
	auto secondInputLayout = Engine::ShaderManager::CreateInputLayout("Second", emissiveShader->vertexBlob.Get(), secondIed, sizeof(secondIed) / sizeof(D3D11_INPUT_ELEMENT_DESC));
	auto thirdLayout = Engine::ShaderManager::CreateInputLayout("Third", NormalVisLines->vertexBlob.Get(), normalIED, sizeof(normalIED) / sizeof(D3D11_INPUT_ELEMENT_DESC));
	auto fourthLayout = Engine::ShaderManager::CreateInputLayout("Fourth", dissolutionShader->vertexBlob.Get(), thirdIed, sizeof(thirdIed) / sizeof(D3D11_INPUT_ELEMENT_DESC));
	auto incinerationLayout = Engine::ShaderManager::CreateInputLayout("incinerationLayout", incenerationShader->vertexBlob.Get(), incinerationIED, sizeof(incinerationIED) / sizeof(D3D11_INPUT_ELEMENT_DESC));
	auto sphereLayout = Engine::ShaderManager::CreateInputLayout("sphereLayout", gpuSphereShader->vertexBlob.Get(), gpuSphereIED, sizeof(gpuSphereIED) / sizeof(D3D11_INPUT_ELEMENT_DESC));

	decalShader->BindInputLyout(decalLayout);
	NormalVisColor->BindInputLyout(thirdLayout);
	NormalVisLines->BindInputLyout(thirdLayout);
	HologramGroup->BindInputLyout(thirdLayout);
	textureMap->BindInputLyout(thirdLayout);
	opaqueShader->BindInputLyout(inputLayout);
	GopaqueShader->BindInputLyout(inputLayout);
	emissiveShader->BindInputLyout(secondInputLayout);
	GemissiveShader->BindInputLyout(secondInputLayout);
	shadowShader->BindInputLyout(thirdLayout);
	shadowShader2->BindInputLyout(thirdLayout);
	shadowShader3->BindInputLyout(fourthLayout);
	shadowShader4->BindInputLyout(fourthLayout);
	dissolutionShader->BindInputLyout(fourthLayout);
	GdissolutionShader->BindInputLyout(fourthLayout);
	incenerationShader->BindInputLyout(incinerationLayout);
	incerShadowPL->BindInputLyout(incinerationLayout);
	incerShadowSLDL->BindInputLyout(incinerationLayout);
	gpuSphereShader->BindInputLyout(sphereLayout);
	Engine::ShadowSystem::Init()->SetShadowShaders(shadowShader, shadowShader2, shadowShader2);
	Engine::DecalSystem::Init()->SetShader(decalShader);
	

	auto ms = Engine::MeshSystem::Init();


	ms->opaqueGroup.setGBufferShader(GopaqueShader);
	ms->emmisiveGroup.setGBufferShader(GemissiveShader);
	ms->dissolutionGroup.setGBufferShader(GdissolutionShader);
	ms->hologramGroup.setGBufferShader(HologramGroup);
	ms->setLitDefferedShader(DefferedOpaqueShader);
	ms->setEmissiveDefferedShader(DefferedEmissiveShader);

	ms->normVisGroup.addShader(NormalVisLines);
	ms->normVisGroup.addShader(NormalVisColor);
	ms->hologramGroup.addShader(HologramGroup);
	ms->hologramGroup.addShader(NormalVisLines);

	ms->opaqueGroup.addShader(opaqueShader);
	ms->opaqueGroup.addShader(NormalVisLines);

	ms->emmisiveGroup.addShader(emissiveShader);

	ms->dissolutionGroup.addShader(dissolutionShader);
	ms->incinerationGroup.setGBufferShader(incenerationShader);
	ms->incinerationGroup.addShader(incenerationShader);

	Engine::ParticleSystem::Init()->SetGPUParticlesShaders(GPUbillboardShader, gpuSphereShader, cs1, cs2);
}

D3DApplication::D3DApplication(int windowWidth, int windowHeight, WinProc windowEvent) :
	pWindow(new Engine::Window(windowWidth, windowHeight, windowEvent)) {
	
	InitCamera(windowWidth, windowHeight);
	InitMeshSystem();
	InitSamuraiModel();
	InitLights();
	InitCrateModel();
	InitSponza();
	InitSkybox();
	InitPostProcess();
	ImGui_ImplWin32_Init(pWindow->getHWND());	
	Engine::MeshSystem::Init()->updateInstanceBuffers();

	Engine::ParticleSystem::Init()->SetSparkTexture(Engine::TextureManager::Init()->LoadFromFile("spark", L"Textures\\spark.dds"));
	Engine::ParticleSystem::Init()->InitGPUParticles();
}


bool D3DApplication::isClosed()
{
	return pWindow->isClosed();
}

void D3DApplication::Update(float deltaTime)
{
	GUI();

	UpdateInput(deltaTime);
	if (pWindow->wasWindowResized())
	{
		camera->calculateProjectionMatrix(pWindow->getWindowWidth(), pWindow->getWindowHeight());
	}
	
	Engine::MeshSystem::Init()->dissolutionGroup.update(deltaTime);
	Engine::MeshSystem::Init()->incinerationGroup.update(deltaTime);
	ShadingGroupSwap();

	Engine::TextureManager::Init()->BindSamplers();
	Engine::ParticleSystem::Init()->Update(deltaTime);

	Engine::Renderer* renderer = Engine::Renderer::GetInstance();
	renderer->updatePerFrameCB(deltaTime, (FLOAT)pWindow->getWindowWidth(), (FLOAT)pWindow->getWindowHeight(), camera->getNearClip(), camera->getFarClip());
	renderer->Render(camera.get());
		
	renderer->PostProcess();
	renderer->FXAA();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	pWindow->flush();
}

void D3DApplication::UpdateInput(float deltaTime)
{
	Engine::vec2 mousePosition = Input::getMousePosition();

	Engine::vec3 cameraMoveDirection = (0.0f, 0.0f, 0.0f);
	if (cameraStates.canMove)
	{
		if (Input::keyIsDown(Input::KeyboardButtons::W))
			cameraMoveDirection += camera->getForward() * deltaTime;
		if (Input::keyIsDown(Input::KeyboardButtons::A))
			cameraMoveDirection += camera->getRight() * -deltaTime;
		if (Input::keyIsDown(Input::KeyboardButtons::S))
			cameraMoveDirection += camera->getForward() * -deltaTime;
		if (Input::keyIsDown(Input::KeyboardButtons::D))
			cameraMoveDirection += camera->getRight() * deltaTime;
		if (Input::keyIsDown(Input::KeyboardButtons::Q))
			cameraMoveDirection += camera->getUp() * -deltaTime;
		if (Input::keyIsDown(Input::KeyboardButtons::E))
			cameraMoveDirection += camera->getUp() * deltaTime;
		if (Input::keyIsDown(Input::KeyboardButtons::SHIFT))
			cameraMoveDirection *= 5;
	}
	if (Input::mouseWasPressed(Input::MouseButtons::LEFT))
		previousMousePosition = mousePosition;
	if (Input::keyIsDown(Input::KeyboardButtons::PLUS))
		Engine::PostProcess::Init()->AddEV100(deltaTime * 2.0f);
	if (Input::keyIsDown(Input::KeyboardButtons::MINUS))
		Engine::PostProcess::Init()->AddEV100(-deltaTime * 2.0f);
	if (Input::keyPresseed(Input::KeyboardButtons::F))
	{
		auto ls = Engine::LightSystem::Init();
		ls->SetFlashLightAttachedState(!ls->IsFlashLightAttached());
	}

	if (Input::keyPresseed(Input::KeyboardButtons::G))
	{
		Engine::ray r;
		Engine::vec2 screenCoord = Engine::screenSpaceToNormalizeScreenSpace(mousePosition, pWindow->getWindowWidth(), pWindow->getWindowHeight());
		r.origin = camera->getPosition();
		r.direction = camera->calculateRayDirection(screenCoord).normalized();

		Engine::hitInfo hInfo; hInfo.reset_parameter_t();
		uint32_t hitId = Engine::MeshSystem::Init()->opaqueGroup.intersectMesh(r, hInfo);
		
		if (hitId != -1)
		{
			Engine::mat4 transfrom = camera->getInverseViewMatrix();
			Engine::quaternion rotaton = Engine::quaternion::angleAxis(get_random(g_distribution_0_2PI), camera->getForward());
			Engine::vec3 rotatatedRight = Engine::quaternion::rotate(rotaton, camera->getRight()).normalized();
			Engine::vec3 up = Engine::cross(camera->getForward(), rotatatedRight);
			transfrom = Engine::transformMatrix(hInfo.p, camera->getForward(), rotatatedRight, up);


			Engine::vec3 albedo = Engine::vec3(get_random(g_distribution_0_2), get_random(g_distribution_0_2), get_random(g_distribution_0_2)) * 0.5f;
			

			Engine::DecalSystem::Init()->AddDecal(transfrom, hitId, decalMaterial,
				albedo, decalRoughMetal.x, decalRoughMetal.y);
		}
	}

	if (Input::keyPresseed(Input::KeyboardButtons::ONE))
		Engine::TextureManager::Init()->BindSampleByFilter(D3D11_FILTER_MIN_MAG_MIP_POINT, 3u);
	else if (Input::keyPresseed(Input::KeyboardButtons::TWO))
		Engine::TextureManager::Init()->BindSampleByFilter(D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, 3u);
	else if (Input::keyPresseed(Input::KeyboardButtons::THREE))
		Engine::TextureManager::Init()->BindSampleByFilter(D3D11_FILTER_ANISOTROPIC, 3u);

	if (Input::keyPresseed(Input::KeyboardButtons::M))
	{
		auto MS = Engine::MeshSystem::Init();
		
		Engine::TransformSystem::transforms transform = { camera->getInverseViewMatrix() };
		(Engine::vec3&)*transform.modelToWold[3] += camera->getForward() * 3;
		switch (modelToSpawn)
		{
		case SAMURAI:
		{
			(Engine::vec3&)*transform.modelToWold[3] += -camera->getUp();
			auto model = Engine::ModelManager::Init()->GetModel("Models\\Samurai.fbx");
			MS->dissolutionGroup.addModel(model, samuraiDisolutionMaterial, transform, Instances::DissolutionInstance{ animationDuration });
			
		}
			break;
		case CRATE:
		{
			auto model = Engine::ModelManager::Init()->GetModel("Models\\cube.obj");
			MS->dissolutionGroup.addModel(model, cubeDisolutionMaterial, transform, Instances::DissolutionInstance{ animationDuration });
		}
			break;
		default:
			break;
		}

		MS->dissolutionGroup.updateInstanceBuffers();
	}


	int scrolls = Input::scrollAmount();
	if (scrolls > 0)
		cameraSpeed += cameraSpeed * 0.1f * scrolls;
	else if (scrolls < 0)
		cameraSpeed += cameraSpeed * 0.1f * scrolls;




	camera->moveCamera(cameraMoveDirection * cameraSpeed);

	bool cameraRotated = false;
	Engine::vec2 delta;
	if (Input::mouseIsDown(Input::MouseButtons::LEFT) && cameraStates.canRotate)
	{
		delta = (mousePosition - previousMousePosition) / Engine::vec2((float)pWindow->getWindowWidth(),(float)pWindow->getWindowHeight()) * (2.0f * (float)M_PI * deltaTime);
		if (delta.x != 0 || delta.y != 0)
		{
			Engine::quaternion q = (Engine::quaternion::angleAxis(delta.y, camera->getRight()) *
			Engine::quaternion::angleAxis(delta.x, camera->getUp())).normalize();
			camera->setForward(Engine::quaternion::rotate(q, camera->getForward()));
			cameraRotated = true;
		}

	}

	if (Input::keyPresseed(Input::KeyboardButtons::DEL))
	{
		Engine::vec2 screenCoord = Engine::screenSpaceToNormalizeScreenSpace(mousePosition, pWindow->getWindowWidth(), pWindow->getWindowHeight());
		Engine::ray r;
		r.origin = camera->getPosition();
		r.direction = camera->calculateRayDirection(screenCoord).normalized();

		Engine::hitInfo hInfo; hInfo.reset_parameter_t();
		uint32_t hitId = Engine::MeshSystem::Init()->opaqueGroup.intersect(r, hInfo);

		if (hitId != -1)
			OpaqueToIncineration(hitId, hInfo.p);
	}

	if (Input::mouseWasPressed(Input::MouseButtons::RIGHT) && objectInteractions == Drag)
	{
		Engine::ray r;
		Engine::vec2 screenCoord = Engine::screenSpaceToNormalizeScreenSpace(mousePosition, pWindow->getWindowWidth(), pWindow->getWindowHeight());
		r.origin = camera->getPosition();
		r.direction = camera->calculateRayDirection(screenCoord).normalized();

		Engine::hitInfo hInfo; hInfo.reset_parameter_t();
		uint32_t hitId = Engine::MeshSystem::Init()->intersect(r, hInfo);

		if(hitId != -1)
			dragger = std::make_unique<Engine::IInstanceDragger>(hitId, hInfo);
				
	}
	else if (!Input::mouseIsDown(Input::MouseButtons::RIGHT))
	{
		dragger.release();
	}

	if (Input::mouseWasPressed(Input::MouseButtons::RIGHT) && objectInteractions == Select)
	{
	
		Engine::ray r;
		Engine::vec2 screenCoord = Engine::screenSpaceToNormalizeScreenSpace(mousePosition, pWindow->getWindowWidth(), pWindow->getWindowHeight());
		r.origin = camera->getPosition();
		r.direction = camera->calculateRayDirection(screenCoord);

		Engine::hitInfo hInfo; hInfo.reset_parameter_t();
		auto& opaqueGroup = Engine::MeshSystem::Init()->opaqueGroup;
		int opaqueHit = opaqueGroup.intersect(r, hInfo);

		auto& emmisiveGroup = Engine::MeshSystem::Init()->emmisiveGroup;
		int emmisiveHit = emmisiveGroup.intersect(r, hInfo);

		if (selected && opaqueHit != selected->getTransformId() && selectedObject == Opaque)
		{
			Instances::PBRInstance data = { false };
			selected->update(&data);
		}

		auto emitter = Engine::ParticleSystem::Init()->getEmitterByTransformId(emmisiveHit);
		if (emitter)
		{
			pEmitter = emitter;
			selectedObject = Emitter;
		}
		else if(emmisiveHit != -1 && Engine::LightSystem::Init()->GetPointLightByTransformId(emmisiveHit))
		{
			selectedObject = Emmisive;
			selected = std::make_unique<Engine::IInstanceSelected<Instances::EmmisiveInstance>>(emmisiveHit, std::move(emmisiveGroup.getInstanceByTransformId(emmisiveHit)));
		}
		else if (opaqueHit != -1)
		{
			selectedObject = Opaque;
			selected = std::make_unique<Engine::IInstanceSelected<Instances::PBRInstance>>(opaqueHit, std::move(opaqueGroup.getInstanceByTransformId(opaqueHit)));
		}
	}
	else if (selected && objectInteractions != Select)
	{
		if (selectedObject == Opaque)
		{
			Instances::PBRInstance data = { false };
			selected->update(&data);
		}
		selected.release();
		pEmitter = nullptr;
	}

	if (cameraMoveDirection != Engine::vec3(0.0f) || cameraRotated)
	{
		camera->calculateViewMatrix();
		camera->setRight(Engine::cross(camera->getUp(), camera->getForward()));
		camera->calculateRayDirections();
	}

	if (dragger)
	{
		if (dragger->isValid())
		{
			Engine::vec2 screenCoord(mousePosition.x, pWindow->getWindowHeight() - mousePosition.y);
			Engine::ray r;
			screenCoord.x = (screenCoord.x / pWindow->getWindowWidth() - 0.5f) * 2.0f;
			screenCoord.y = (screenCoord.y / pWindow->getWindowHeight() - 0.5f) * 2.0f;
			r.origin = camera->getPosition();
			r.direction = camera->calculateRayDirection(screenCoord).normalized();

			dragger->drag(r);
		}
		else
		{
			dragger.release();
		}
	}

}

D3DApplication::~D3DApplication()
{
}

void D3DApplication::GUI()
{
	Engine::Renderer* renderer = Engine::Renderer::GetInstance();

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");


	if (ImGui::BeginTabBar("Light computation states"))
	{

		if (ImGui::BeginTabItem("Light"))
		{
			ImGui::Checkbox("Diffuse State", &renderer->getDiffuseState());
			ImGui::Checkbox("Specular State", &renderer->getSpecularState());
			ImGui::Checkbox("IBL State", &renderer->getIBLLghtState());
			ImGui::Checkbox("LTC State", &renderer->getLTCState());

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Camera"))
		{
			ImGui::Checkbox("Can move", &cameraStates.canMove);
			ImGui::Checkbox("Can rotate", &cameraStates.canRotate);

			ImGui::EndTabItem();
		}


		if (ImGui::BeginTabItem("Object Interactions"))
		{
			// New section for mode selection and conditional rendering
			ImGui::Separator();
			ImGui::Text("Mode Selection");

			static const char* items[] = { "Drag", "Select" };
			static int currentItem = 0; // 0 for Drag, 1 for Select

			if (ImGui::Combo("Mode", &currentItem, items, IM_ARRAYSIZE(items)))
			{
				// Update the current mode based on selection
				objectInteractions = (Mode)currentItem;
			}
			
			static float roughness = 1.0f;
			static float metalness = 1.0f;
			static bool overwrite = false;

			switch (objectInteractions)
			{
			case D3DApplication::Drag:
				ImGui::Text("Drag Mode");
				if (dragger)
				{
					const Engine::vec3& objectPosition = (Engine::vec3&)*Engine::TransformSystem::Init()->GetModelTransforms(dragger->getObjectID())[0].modelToWold[3];
					ImGui::Text("Object Position: %.2f, %.2f, %.2f", objectPosition.x, objectPosition.y, objectPosition.z);
				}
				break;
			case D3DApplication::Select:
				ImGui::Text("Select Mode");
				
				if (selected || pEmitter)
				{
					switch (selectedObject)
					{
					case D3DApplication::Opaque:
					{
						ImGui::Checkbox("Overwrite Roughness and Metalness of texture", &overwrite);
						if (overwrite)
						{
							roughness = roughness > 1.0f ? 1.0f : roughness;
							metalness = metalness > 1.0f ? 1.0f : metalness;
							ImGui::SliderFloat("Roughness", &roughness, 0.05f, 1.0f);
							ImGui::SliderFloat("Metalness", &metalness, 0.05f, 1.0f);
						}
						else
						{
							ImGui::SliderFloat("Roughness", &roughness, 0.05f, 10.0f);
							ImGui::SliderFloat("Metalness", &metalness, 0.05f, 10.0f);
						}

						if (ImGui::Button("Reset"))
						{
							roughness = 1.0f;
							metalness = 1.0f;
							overwrite = false;
						}

						auto instance = Instances::PBRInstance{ true, overwrite, roughness, metalness };
						selected->update((void*)&instance);
						break;
					}
					case D3DApplication::Emmisive:
					{
						Engine::PointLight* pl = Engine::LightSystem::Init()->GetPointLightByTransformId(selected->getTransformId());

						float previousRadius = pl->radius;

						ImGui::InputFloat("Radius", &pl->radius);
						ImGui::InputFloat3("Color", (float*)&pl->radiance);

						pl->radius = pl->radius > 0.0f ? pl->radius : 0.01f;

						pl->radiance.x = pl->radiance.x >= 0.0f ? pl->radiance.x : 0.01f;
						pl->radiance.y = pl->radiance.y >= 0.0f ? pl->radiance.y : 0.01f;
						pl->radiance.z = pl->radiance.z >= 0.0f ? pl->radiance.z : 0.01f;

						selected->update((void*)&pl->radiance);
						if (previousRadius != pl->radius)
						{
							Engine::TransformSystem::Init()->ScaleModelTransform(selected->getTransformId(), pl->radius / previousRadius);
						}
					
						break;
					}
					case D3DApplication::Emitter:
					{
						static float particleMaxLifeTime = pEmitter->getParticleLifeTime();
						static float spawnRate = pEmitter->getParticleSpawnRate();
						static Engine::vec2 size = pEmitter->getParticleSize();
						static Engine::vec3 maxSpeed = pEmitter->getParticleMaxSpeed();
						static Engine::vec3 color = pEmitter->getParticleColor();

						ImGui::SliderFloat("Life time", &particleMaxLifeTime, 0.05f, 10.0f);
						ImGui::InputFloat("Spawn rate", &spawnRate);
						ImGui::InputFloat2("Particle size", (float*)&size);
						ImGui::InputFloat3("Max speed", (float*)&maxSpeed);
						ImGui::InputFloat3("Color",(float*)&color);

						Engine::clamp(size);
						Engine::clamp(maxSpeed);
						Engine::clamp(color);
						maxSpeed = Engine::clampVec(maxSpeed, 3.0f);

						spawnRate = spawnRate < 0.1f ? 0.1f : spawnRate;

						pEmitter->adjustParameters(maxSpeed, size, spawnRate, particleMaxLifeTime);
						pEmitter->setColor(color);

						Engine::MeshSystem::Init()->emmisiveGroup.getInstanceByTransformId(pEmitter->getBindedTransformId())[0]->emmisiveColor = color;
						Engine::MeshSystem::Init()->emmisiveGroup.updateInstanceBuffers();
						break;
					}
					default:
						break;
					}
				}
				break;
			default:
				break;
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Model Spawning"))
		{
			// New section for mode selection and conditional rendering
			ImGui::Separator();
			ImGui::Text("Model to spawn");

			static const char* items[] = { "Samurai", "Crate" };
			static int currentItem = 0; // 0 for Drag, 1 for Select

			if (ImGui::Combo("Mode", &currentItem, items, IM_ARRAYSIZE(items)))
			{
				// Update the current mode based on selection
				modelToSpawn = (objectToSpawn)currentItem;
			}

			ImGui::SliderFloat("Spawn duration", &animationDuration, 0.05f, 10.0f);

			ImGui::EndTabItem();
		}


		if (ImGui::BeginTabItem("Decal"))
		{
			ImGui::SliderFloat("Roughness", &decalRoughMetal.x, 0.05f, 1.0f);
			ImGui::SliderFloat("Metalness", &decalRoughMetal.y, 0.05f, 1.0f);

			/*ImGui::InputFloat3("Color", (float*)&decalAlbedo);*/

			ImGui::EndTabItem();
		}

		static float qualitySubpix = 0.75f;
		static float qualityEdgeThreshold = 0.166f;
		static float qualityEdgeThresholdMin = 0.0833f;
		if (ImGui::BeginTabItem("FXAA"))
		{
			ImGui::SliderFloat("Sub Pixel quality", &qualitySubpix, 0.f, 1.0f);
			ImGui::SliderFloat("Edge Threshold", &qualityEdgeThreshold, 0.063f, 0.333f);

			ImGui::SliderFloat("Minimal Edge Threshold", &qualityEdgeThresholdMin, 0.0f, 0.0833f);

			Engine::PostProcess::Init()->UpdateFXAABuffer(qualitySubpix, qualityEdgeThreshold, qualityEdgeThresholdMin);

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

void D3DApplication::ShadingGroupSwap()
{
	auto endedAnimationInstancesId = Engine::MeshSystem::Init()->dissolutionGroup.checkTimer();

	for (size_t i = 0; i < endedAnimationInstancesId.size(); i++)
	{
		auto modelInstanceData = Engine::MeshSystem::Init()->dissolutionGroup.removeByTransformId(endedAnimationInstancesId[i], false);
		std::vector<Materials::OpaqueTextureMaterial> opaqueMaterial;
		opaqueMaterial.reserve(modelInstanceData.material.size());

		for (size_t j = 0; j < modelInstanceData.material.size(); j++)
		{
			opaqueMaterial.push_back(modelInstanceData.material[j].opaqueTextures);
		}

		Engine::MeshSystem::Init()->opaqueGroup.addModel(modelInstanceData.model, opaqueMaterial, endedAnimationInstancesId[i]);
		Engine::MeshSystem::Init()->opaqueGroup.updateInstanceBuffers();
	}
}

void D3DApplication::OpaqueToIncineration(uint32_t transformId, const Engine::vec3& spherePos)
{
	auto modelInstanceData = Engine::MeshSystem::Init()->opaqueGroup.removeByTransformId(transformId, false);
	Engine::OpaqueInstances<Instances::IncinerationInstance, Materials::DissolutionMaterial>::ModelInstanceData data;
	data.instance.resize(modelInstanceData.instance.size());
	data.material.resize(modelInstanceData.material.size());
	data.model = modelInstanceData.model;

	Engine::vec3 randomColor = Engine::vec3(get_random(g_distribution_0_1), get_random(g_distribution_0_1), get_random(g_distribution_0_1));
	auto TS = Engine::TransformSystem::Init();
	for (size_t j = 0; j < modelInstanceData.instance.size(); j++)
	{
		data.material[j].opaqueTextures = modelInstanceData.material[j];
		data.material[j].noiseTexture = samuraiDisolutionMaterial[0].noiseTexture;
		data.instance[j].instanceMeshId = modelInstanceData.instance[j].instanceMeshId;
		data.instance[j].transformsId = modelInstanceData.instance[j].transformsId;
		

		data.instance[j].instanceData = Instances::IncinerationInstance{ spherePos - (Engine::vec3&)*TS->GetModelTransforms(data.instance[j].transformsId)[j].modelToWold[3] , randomColor.normalized()};
	}

	Engine::MeshSystem::Init()->incinerationGroup.addModel(data);
	Engine::MeshSystem::Init()->incinerationGroup.updateInstanceBuffers();

}

void D3DApplication::InitCamera(int windowWidth, int windowHeight)
{
	camera.reset(new Engine::Camera(45.0f, 0.01f, 100.0f));
	camera->calculateProjectionMatrix(windowWidth, windowHeight);
	camera->calculateRayDirections();
}

void D3DApplication::InitSamuraiModel()
{
	auto TM = Engine::TextureManager::Init();

	auto noiseTexture = Engine::TextureManager::Init()->LoadFromFile("noise", L"Textures\\Noise_19.dds");

	std::shared_ptr<Engine::Texture> emptyTexture = std::make_shared<Engine::Texture>();
	std::vector<Materials::OpaqueTextureMaterial> samuraiTextures = {
		{ TM->LoadFromFile("samurai_sword", L"Textures\\Samurai\\Sword_BaseColor.dds"),
		  TM->LoadFromFile("samurai_sword_roughness", L"Textures\\Samurai\\Sword_Roughness.dds"),
		  TM->LoadFromFile("samurai_sword_metallic", L"Textures\\Samurai\\Sword_Metallic.dds"),
		  TM->LoadFromFile("samurai_sword_normal", L"Textures\\Samurai\\Sword_Normal.dds") },
		{ TM->LoadFromFile("samurai_head", L"Textures\\Samurai\\Head_BaseColor.dds"),
		  TM->LoadFromFile("samurai_head_rougness", L"Textures\\Samurai\\Head_Roughness.dds"),
		  emptyTexture,
		  TM->LoadFromFile("samurai_head_normal", L"Textures\\Samurai\\Head_Normal.dds") },
		{ TM->LoadFromFile("samurai_eyes", L"Textures\\Samurai\\Eyes_BaseColor.dds"),
		  emptyTexture, emptyTexture,
		  TM->LoadFromFile("samurai_eyes_normal", L"Textures\\Samurai\\Eyes_Normal.dds"), 0, 0.05f, 0.08f },
		{ TM->LoadFromFile("samurai_helmet", L"Textures\\Samurai\\Helmet_BaseColor.dds"),
		  TM->LoadFromFile("samurai_helmet_rougness", L"Textures\\Samurai\\Helmet_Roughness.dds"),
		  TM->LoadFromFile("samurai_helmet_metallic", L"Textures\\Samurai\\Helmet_Metallic.dds"),
		  TM->LoadFromFile("samurai_helmet_normal", L"Textures\\Samurai\\Helmet_Normal.dds") },
		{ TM->LoadFromFile("samurai_decor", L"Textures\\Samurai\\Decor_BaseColor.dds"),
		  TM->LoadFromFile("samurai_decor_roughness", L"Textures\\Samurai\\Decor_Roughness.dds"),
		  TM->LoadFromFile("samurai_decor_metallic", L"Textures\\Samurai\\Decor_Metallic.dds"),
		  TM->LoadFromFile("samurai_decor_normal", L"Textures\\Samurai\\Decor_Normal.dds") },
		{ TM->LoadFromFile("samurai_pants", L"Textures\\Samurai\\Pants_BaseColor.dds"),
		  TM->LoadFromFile("samurai_pants_roughness", L"Textures\\Samurai\\Pants_Roughness.dds"),
		  TM->LoadFromFile("samurai_pants_metalness", L"Textures\\Samurai\\Pants_Metallic.dds"),
		  TM->LoadFromFile("samurai_pants_normal", L"Textures\\Samurai\\Pants_Normal.dds") },
		{ TM->LoadFromFile("samurai_hands", L"Textures\\Samurai\\Hands_BaseColor.dds"),
		  TM->LoadFromFile("samurai_hands_roughness", L"Textures\\Samurai\\Hands_Roughness.dds"),
		  emptyTexture,
		  TM->LoadFromFile("samurai_hands_normal", L"Textures\\Samurai\\Hands_Normal.dds") },
		{ TM->LoadFromFile("samurai_torso", L"Textures\\Samurai\\Torso_BaseColor.dds"),
		  TM->LoadFromFile("samurai_torso_roughness", L"Textures\\Samurai\\Torso_Roughness.dds"),
		  TM->LoadFromFile("samurai_torso_metallic", L"Textures\\Samurai\\Torso_Metallic.dds"),
		  TM->LoadFromFile("samurai_torso_normal", L"Textures\\Samurai\\Torso_Normal.dds") }
	};

	samuraiDisolutionMaterial.reserve(samuraiTextures.size());
	for (size_t i = 0; i < samuraiTextures.size(); i++)
	{
		samuraiDisolutionMaterial.push_back({ samuraiTextures[i], noiseTexture });
	}

	auto model = Engine::ModelManager::GetInstance()->loadModel("Models\\NeoNCat.fbx", false, nullptr, true);
	 model = Engine::ModelManager::GetInstance()->loadModel("Models\\Samurai.fbx");
	Engine::TransformSystem::transforms inst = {
		Engine::transformMatrix(Engine::vec3(0.0f, -1.0f, 0.0f), Engine::vec3(0.0f, 0.0f, 1.0f), Engine::vec3(1.0f, 0.0f, 0.0f), Engine::vec3(0.0f, 1.0f, 0.0f)) };
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, samuraiTextures, inst);

	changepos(inst, Engine::vec3(4.0f, -3.0f, 0.0f));
	Engine::MeshSystem::Init()->hologramGroup.addModel(model, Materials::HologramMaterial{ Engine::vec3(0,0,1), 0, Engine::vec3(1,0,0) }, inst);
	auto samuraiInstance = inst;


	changescale(samuraiInstance, 0, 1.5f); changescale(samuraiInstance, 1, 1.0f); changescale(samuraiInstance, 2, 0.5f);
	changepos(samuraiInstance, Engine::vec3(4.0f, -1.0f, 0.0f));
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, samuraiTextures, samuraiInstance);

	samuraiInstance = inst;
	changepos(samuraiInstance, Engine::vec3(8.0f, 1.0f, 0.0f));
	changescale(samuraiInstance, 0, 2.0f); changescale(samuraiInstance, 1, 2.0f); changescale(samuraiInstance, 2, 2.0f);
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, samuraiTextures, samuraiInstance);

	samuraiInstance = inst;
	changepos(samuraiInstance, Engine::vec3(6.0f, 0.0f, 3.0f));
	changescale(samuraiInstance, 0, 1.5f); changescale(samuraiInstance, 1, 0.5f); changescale(samuraiInstance, 2, 1.5f);
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, samuraiTextures, samuraiInstance);
}

void D3DApplication::InitLights()
{
	auto TM = Engine::TextureManager::Init();

	Engine::ModelManager::GetInstance()->initUnitSphere();
	auto model = Engine::ModelManager::GetInstance()->GetModel("UNIT_SPHERE");

	Engine::LightSystem::Init()->AddPointLight(Engine::vec3(1.5f, 0.0f, 0.9f), 1.0f, 3.0f, Engine::vec3(-5.0f, 0.0f, 2.0f), model);
	Engine::LightSystem::Init()->AddPointLight(Engine::vec3(2.35f), 0.5f, 1.84f, Engine::vec3(2.0f, -1.0f, 0.0f), model);
	Engine::LightSystem::Init()->AddPointLight(Engine::vec3(0.0f, 4.0f, 0.0f), 0.5f, 2.25f, Engine::vec3(2.0f, 2.0f, 0.0f), model);
	Engine::LightSystem::Init()->AddPointLight(Engine::vec3(0.1f), 0.05f, 1.4f, Engine::vec3(0.0f, 0.0f, -0.5f), model);

	Engine::SpotLight spotLight(Engine::vec3(1.0f), 1.0f, 11.18f, Engine::vec3(0.0f, 0.0f, 0.0f), Engine::vec3(.0f, .0f, 1.0f), 0.5 / 2.0f);
	spotLight.bindedObjectId = camera->getCameraTransformId();
	Engine::LightSystem::Init()->AddFlashLight(spotLight, TM->LoadFromFile("flashlight", L"Textures\\flashlightMask.dds"));

	Engine::DirectionalLight directionalLight(Engine::vec3(-0.3205475307f, -0.595605361f, -0.10348193f).normalized(), Engine::vec3(0.84f * 7.5f, 0.86264f * 7.5f, 0.89019f * 7.5f), 0.2f);
	Engine::LightSystem::Init()->AddDirectionalLight(directionalLight);

	Engine::TransformSystem::transforms bombo = {
		Engine::transformMatrix(Engine::vec3(0.0f, 8.0f, 0.0f), Engine::vec3(0.0f, 0.0f, 0.1f), Engine::vec3(0.1f, 0.0f, 0.0f), Engine::vec3(0.0f, 0.1f, 0.0f)) };

	auto id = Engine::MeshSystem::Init()->emmisiveGroup.addModel(model, Materials::EmmisiveMaterial{}, bombo, Instances::EmmisiveInstance{ Engine::vec3(1.0f) });
	Engine::Emitter emitter(Engine::vec3(0.0f), id, 50, 0.1f, Engine::vec3(1.0f));
	Engine::ParticleSystem::Init()->addSmokeEmitter(emitter);
	changepos(bombo, Engine::vec3(8.0f, -2.0f, 0.0f));
	id = Engine::MeshSystem::Init()->emmisiveGroup.addModel(model, Materials::EmmisiveMaterial{}, bombo, Instances::EmmisiveInstance{ Engine::vec3(1.0f,0.0f,1.0f) });
	Engine::Emitter emitter2(Engine::vec3(0.0f), id, 50, 0.1f, Engine::vec3(1.0f,0.0f,1.0f));
	emitter2.adjustParameters(Engine::vec3(0.1f, 0.4f, 0.1f), Engine::vec2(0.5f), 50, 10.0f);
	Engine::ParticleSystem::Init()->addSmokeEmitter(emitter2);
	auto EMVA = TM->LoadFromFile("EMVA", L"Textures\\Smoke\\smoke_MVEA.dds");
	auto RLU = TM->LoadFromFile("RLU", L"Textures\\Smoke\\smoke_RLU.dds");
	auto DBF = TM->LoadFromFile("DBF", L"Textures\\Smoke\\smoke_DBF.dds");
	Engine::ParticleSystem::Init()->SetSmokeTextures(RLU, DBF, EMVA);

	Engine::vec3 vert[4] = { Engine::vec3(-1.0f, 1.0f, 0.0f),  Engine::vec3(1.0f, 1.0f, 0.0f), Engine::vec3(1.0f, -1.0f, 0.0f), Engine::vec3(-1.0f, -1.0f, 0.0f) };
	Engine::AreaLight areaLight(Engine::vec3(0.0f, .3f, 0.7f), vert, 4, 10.0f);
	Engine::ModelManager::GetInstance()->initUnitQuad();
	model = Engine::ModelManager::GetInstance()->GetModel("UNIT_QUAD");
	Engine::TransformSystem::transforms inst = {
	   Engine::transformMatrix(Engine::vec3(-2.0f, 3.0f, 7.0f), Engine::vec3(0.0f, 0.0f, 1.0f), Engine::vec3(1.0f, 0.0f, 0.0f), Engine::vec3(0.0f, 1.0f, 0.0f))};

	areaLight.bindedTransform = Engine::MeshSystem::Init()->emmisiveGroup.addModel(model, Materials::EmmisiveMaterial{}, inst, Instances::EmmisiveInstance{ areaLight.radiance });
	Engine::LightSystem::Init()->AddAreaLight(areaLight);

	Engine::LightSystem::Init()->UpdateLightsBuffer();

	auto LTCmat = Engine::TextureManager::Init()->LoadFromFile("LTCmat", L"Textures\\ltc_mat.dds");
	auto LTCamp = Engine::TextureManager::Init()->LoadFromFile("LTCamp", L"Textures\\ltc_amp.dds");

	Engine::Renderer::GetInstance()->setLTCLight(LTCmat, LTCamp);


	auto diffuse = Engine::TextureManager::Init()->LoadFromFile("IBLd", L"Textures\\PreCalculatedIBL\\diffuse.dds");
	auto specular = Engine::TextureManager::Init()->LoadFromFile("IBLs", L"Textures\\PreCalculatedIBL\\specIrrad.dds");
	auto reflectance = Engine::TextureManager::Init()->LoadFromFile("IBLr", L"Textures\\PreCalculatedIBL\\reflectance.dds");	

	Engine::Renderer::GetInstance()->setIBLLight(diffuse, specular, reflectance);
}

void D3DApplication::InitCrateModel()
{
	auto TM = Engine::TextureManager::Init();

	Materials::OpaqueTextureMaterial goldenSphereTextures = { TM->LoadFromFile("golden_albedo", L"Textures\\Gold\\albedo.dds"),
		TM->LoadFromFile("golden_roughness", L"Textures\\Gold\\roughness.dds"), TM->LoadFromFile("golden_metallic", L"Textures\\Gold\\metallic.dds"),
		TM->LoadFromFile("golden_normal",L"Textures\\Gold\\normal.dds") };


	auto model = Engine::ModelManager::GetInstance()->loadModel("Models\\sphere.obj");
	Engine::TransformSystem::transforms inst = { Engine::transformMatrix(Engine::vec3(0.0f, 4.0f, -1.0f), Engine::vec3(0.0f, 0.0f, 1.0f), Engine::vec3(1.0f, 0.0f, 0.0f), Engine::vec3(0.0f, 1.0f, 0.0f)) };
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, goldenSphereTextures, inst);
	

	model = Engine::ModelManager::GetInstance()->loadModel("Models\\cube.obj", true);
	auto crateFirst = TM->LoadFromFile("crate", L"Textures\\RedCore\\albedo.dds");
	auto crateMetallic = TM->LoadFromFile("crateMetallic", L"Textures\\RedCore\\metallic.dds");
	auto crateRoughness = TM->LoadFromFile("crateRoughness", L"Textures\\RedCore\\roughness.dds");
	auto crateNormal = TM->LoadFromFile("crateNormal", L"Textures\\RedCore\\normal.dds");

	auto noiseTexture = TM->GetTexture("noise");
	Materials::OpaqueTextureMaterial crateMaterial = { crateFirst, crateRoughness, crateMetallic, crateNormal };

	cubeDisolutionMaterial = { crateMaterial, noiseTexture };

	changepos(inst, Engine::vec3(1.0f, -1.0f, 4.0f));
	auto issd = Engine::MeshSystem::Init()->opaqueGroup.addModel(model, crateMaterial, inst);


	auto goldenCube = goldenSphereTextures;
	goldenCube.usedTextures = Materials::METALNESS;
	auto emptyTexture = std::make_shared<Engine::Texture>();
	changepos(inst, Engine::vec3(-3.0f, -1.0f, 2.0f));
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, goldenCube, inst);

	decalMaterial = Materials::DecalMaterial{ emptyTexture , TM->LoadFromFile("Decal_Normal", L"Textures\\DecalNormal.dds"), emptyTexture , emptyTexture , Materials::NORMAL };

	auto rotZ = Engine::mat4::rotateZ(3.14f * (-45.0f) / 360.0f);
	changescale(inst, 0, 5);
	changepos(inst, Engine::vec3(-10.0f, 4.0f, 1.2f));
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, crateMaterial, Engine::TransformSystem::transforms{ inst.modelToWold * rotZ });
}

void D3DApplication::InitFloor()
{
	auto TM = Engine::TextureManager::Init();
	auto model = Engine::ModelManager::GetInstance()->loadModel("Models\\cube.obj", true);

	auto floorAlbedo = TM->LoadFromFile("floorAlbedo", L"Textures\\Concrete\\albedo.dds");
	auto floorRoughness = TM->LoadFromFile("floorRoughness", L"Textures\\Concrete\\roughness.dds");
	auto floorMetallic = TM->LoadFromFile("floorMetallic", L"Textures\\Concrete\\metallic.dds");
	auto floorNormal = TM->LoadFromFile("floorNormal", L"Textures\\Concrete\\normal.dds");

	Materials::OpaqueTextureMaterial floorMaterial = { floorAlbedo, floorRoughness, floorMetallic, floorNormal };

	Engine::TransformSystem::transforms inst = {
		Engine::transformMatrix(Engine::vec3(0.0f, -1.0f, 0.0f), Engine::vec3(0.0f, 0.0f, 1.0f), Engine::vec3(1.0f, 0.0f, 0.0f), Engine::vec3(0.0f, 1.0f, 0.0f)) };

	for (int i = 0; i < 100; i += 2) {
		for (int j = 0; j < 100; j += 2) {
			changepos(inst, Engine::vec3(-50.0f + (float)i, -5.0f, -50.0f + (float)j));
			Engine::MeshSystem::Init()->opaqueGroup.addModel(model, floorMaterial, inst);
		}
	}
}

void D3DApplication::InitSponza()
{
	auto TM = Engine::TextureManager::Init();

	Engine::TransformSystem::transforms inst = { Engine::transformMatrix(Engine::vec3(0.0f, -5.0f, 0.0f), Engine::vec3(0.0f, 0.0f, 0.025f), Engine::vec3(0.025, 0.0f, 0.0f), Engine::vec3(0.0f,  0.025, 0.0f)) };
	auto emptyTexture = std::make_shared<Engine::Texture>();

	std::vector<Materials::OpaqueTextureMaterial> sponzaTextures = {

		// vase - Vase Hanging
		{ TM->LoadFromFile("vase_hanging_diffuse", L"Textures\\Sponza\\VaseHanging_diffuse.dds"),
		TM->LoadFromFile("vase_hanging_roughness", L"Textures\\Sponza\\VaseHanging_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("vase_hanging_normal", L"Textures\\Sponza\\VaseHanging_normal.dds") },

		// Material__25
		{ TM->LoadFromFile("lion_diffuse", L"Textures\\Sponza\\Lion_Albedo.dds"),
		TM->LoadFromFile("lion_roughness", L"Textures\\Sponza\\Lion_Roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("lion_normal", L"Textures\\Sponza\\Lion_Normal.dds") },


		// Background
		{ TM->LoadFromFile("background_diffuse", L"Textures\\Sponza\\Background_Albedo.dds"),
		TM->LoadFromFile("background_roughness", L"Textures\\Sponza\\Background_Roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("background_normal", L"Textures\\Sponza\\Background_Normal.dds") },

		// Material__47 (no maps listed)
		{ emptyTexture, emptyTexture, emptyTexture, emptyTexture },

		// Material__57
		{ TM->LoadFromFile("vase_plant_diffuse", L"Textures\\Sponza\\VasePlant_diffuse.dds"),
		TM->LoadFromFile("vase_plant_roughness", L"Textures\\Sponza\\VasePlant_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("vase_plant_normal", L"Textures\\Sponza\\VasePlant_normal.dds") },

		// Material arch
		{ TM->LoadFromFile("arch_diffuse", L"Textures\\Sponza\\Sponza_Arch_diffuse.dds"),
		TM->LoadFromFile("arch_roughness", L"Textures\\Sponza\\Sponza_Arch_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("arch_normal", L"Textures\\Sponza\\Sponza_Arch_normal.dds") },

		// Material bricks
		{ TM->LoadFromFile("bricks_diffuse", L"Textures\\Sponza\\Sponza_Bricks_a_Albedo.dds"),
		TM->LoadFromFile("bricks_roughness", L"Textures\\Sponza\\Sponza_Bricks_a_Roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("bricks_normal", L"Textures\\Sponza\\Sponza_Bricks_a_Normal.dds") },

		// Material ceiling
		{ TM->LoadFromFile("ceiling_diffuse", L"Textures\\Sponza\\Sponza_Ceiling_diffuse.dds"),
		TM->LoadFromFile("ceiling_roughness", L"Textures\\Sponza\\Sponza_Ceiling_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("ceiling_normal", L"Textures\\Sponza\\Sponza_Ceiling_normal.dds") },

		// Material chain
		{ TM->LoadFromFile("chain_diffuse", L"Textures\\Sponza\\ChainTexture_Albedo.dds"),
		TM->LoadFromFile("chain_roughness", L"Textures\\Sponza\\ChainTexture_Roughness.dds"),
		TM->LoadFromFile("chain_metallic", L"Textures\\Sponza\\ChainTexture_Metallic.dds"),
		TM->LoadFromFile("chain_normal", L"Textures\\Sponza\\ChainTexture_Normal.dds") },

		// Material column_a
		{ TM->LoadFromFile("column_a_diffuse", L"Textures\\Sponza\\Sponza_Column_a_diffuse.dds"),
		TM->LoadFromFile("column_a_roughness", L"Textures\\Sponza\\Sponza_Column_a_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("column_a_normal", L"Textures\\Sponza\\Sponza_Column_a_normal.dds") },

		// Material column_b
		{ TM->LoadFromFile("column_b_diffuse", L"Textures\\Sponza\\Sponza_Column_b_diffuse.dds"),
		TM->LoadFromFile("column_b_roughness", L"Textures\\Sponza\\Sponza_Column_b_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("column_b_normal", L"Textures\\Sponza\\Sponza_Column_b_normal.dds") },


		// Material column_c
		{ TM->LoadFromFile("column_c_diffuse", L"Textures\\Sponza\\Sponza_Column_c_diffuse.dds"),
		TM->LoadFromFile("column_c_roughness", L"Textures\\Sponza\\Sponza_Column_c_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("column_c_normal", L"Textures\\Sponza\\Sponza_Column_c_normal.dds") },


		// Material details
		{ TM->LoadFromFile("details_diffuse", L"Textures\\Sponza\\Sponza_Details_diffuse.dds"),
		TM->LoadFromFile("details_roughness", L"Textures\\Sponza\\Sponza_Details_roughness.dds"),
		TM->LoadFromFile("details_metallic", L"Textures\\Sponza\\Sponza_Details_metallic.dds"),
		TM->LoadFromFile("details_normal", L"Textures\\Sponza\\Sponza_Details_normal.dds") },

		// fabric_b - Sponza Fabric Green
		{ TM->LoadFromFile("sponza_fabric_green_diffuse", L"Textures\\Sponza\\Sponza_Fabric_Green_diffuse.dds"),
		TM->LoadFromFile("sponza_fabric_roughness", L"Textures\\Sponza\\Sponza_Fabric_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("sponza_fabric_green_normal", L"Textures\\Sponza\\Sponza_Fabric_Green_normal.dds") },

		// curtain_red - Red Curtain
		{ TM->LoadFromFile("curtain_red_diffuse", L"Textures\\Sponza\\Sponza_Curtain_Red_diffuse.dds"),
		TM->LoadFromFile("curtain_roughness", L"Textures\\Sponza\\Sponza_Curtain_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("curtain_red_normal", L"Textures\\Sponza\\Sponza_Curtain_Red_normal.dds") },
		
		// fabric_a - Sponza Fabric Blue
		{ TM->LoadFromFile("sponza_fabric_blue_diffuse", L"Textures\\Sponza\\Sponza_Fabric_Blue_diffuse.dds"),
		TM->LoadFromFile("sponza_fabric_roughness", L"Textures\\Sponza\\Sponza_Fabric_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("sponza_fabric_blue_normal", L"Textures\\Sponza\\Sponza_Fabric_Blue_normal.dds") },


		// fabric_c - Sponza Fabric Red
		{ TM->LoadFromFile("sponza_fabric_red_diffuse", L"Textures\\Sponza\\Sponza_Fabric_Red_diffuse.dds"),
		TM->LoadFromFile("sponza_fabric_roughness", L"Textures\\Sponza\\Sponza_Fabric_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("sponza_fabric_red_normal", L"Textures\\Sponza\\Sponza_Fabric_Red_normal.dds") },

		// Curtain - Green
		{ TM->LoadFromFile("sponza_curtain_green_diffuse", L"Textures\\Sponza\\Sponza_Curtain_Green_diffuse.dds"),
		TM->LoadFromFile("sponza_curtain_roughness", L"Textures\\Sponza\\Sponza_Curtain_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("sponza_curtain_green_normal", L"Textures\\Sponza\\Sponza_Curtain_Green_normal.dds") },

		// curtain_blue - Blue Curtain
		{ TM->LoadFromFile("curtain_blue_diffuse", L"Textures\\Sponza\\Sponza_Curtain_Blue_diffuse.dds"),
		TM->LoadFromFile("curtain_roughness", L"Textures\\Sponza\\Sponza_Curtain_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("curtain_blue_normal", L"Textures\\Sponza\\Sponza_Curtain_Blue_normal.dds") },

		// Flag - flag
		{ TM->LoadFromFile("flag_albedo", L"Textures\\Sponza\\Sponza_FlagPole_diffuse.dds"),
		TM->LoadFromFile("flag_roughness", L"Textures\\Sponza\\Sponza_FlagPole_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("flag_normal", L"Textures\\Sponza\\Sponza_FlagPole_normal.dds"), },

		// floor - Sponza Floor
		{ TM->LoadFromFile("sponza_floor_diffuse", L"Textures\\Sponza\\Sponza_Floor_diffuse.dds"),
		TM->LoadFromFile("sponza_floor_roughness", L"Textures\\Sponza\\Sponza_Floor_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("sponza_floor_normal", L"Textures\\Sponza\\Sponza_Floor_normal.dds") },



		// Thorne - Thorne
		{ TM->LoadFromFile("thorn_diffuse", L"Textures\\Sponza\\Sponza_Thorn_diffuse.dds"),
		TM->LoadFromFile("thorn_diffuse_roughness", L"Textures\\Sponza\\Sponza_Thorn_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("thorn_diffuse_normal", L"Textures\\Sponza\\Sponza_Thorn_normal.dds") },
	
		// roof - Sponza Roof
		{ TM->LoadFromFile("sponza_roof_diffuse", L"Textures\\Sponza\\Sponza_Roof_diffuse.dds"),
		TM->LoadFromFile("sponza_roof_roughness", L"Textures\\Sponza\\Sponza_Roof_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("sponza_roof_normal", L"Textures\\Sponza\\Sponza_Roof_normal.dds") },


		// vase - Vase
		{ TM->LoadFromFile("vase_diffuse", L"Textures\\Sponza\\Vase_diffuse.dds"),
		TM->LoadFromFile("vase_roughness", L"Textures\\Sponza\\Vase_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("vase_normal", L"Textures\\Sponza\\Vase_normal.dds") },

		// vase - Vase Round
		{ TM->LoadFromFile("vase_round_diffuse", L"Textures\\Sponza\\VaseRound_diffuse.dds"),
		TM->LoadFromFile("vase_round_roughness", L"Textures\\Sponza\\VaseRound_roughness.dds"),
		emptyTexture,
		TM->LoadFromFile("vase_round_normal", L"Textures\\Sponza\\VaseRound_normal.dds") },

		};

	std::vector<uint32_t> materialIndexes;
	auto model = Engine::ModelManager::GetInstance()->loadModel("Models\\sponza.obj", true, &materialIndexes);

	std::vector<Materials::OpaqueTextureMaterial> sponzaMaterials;
	sponzaMaterials.resize(materialIndexes.size());
	for (size_t i = 0; i < sponzaMaterials.size(); i++)
	{
		sponzaMaterials[i] = sponzaTextures[materialIndexes[i] % sponzaTextures.size()];
	}
	inst.modelToWold = inst.modelToWold * Engine::mat4::rotateY(M_PI_2);
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, sponzaMaterials, inst);
	Engine::MeshSystem::Init()->opaqueGroup.addIntersectIgnoredModel(model);
}

void D3DApplication::InitSkybox()
{
	auto skyboxShader = Engine::ShaderManager::CompileAndCreateShader("skybox", L"shaders/skyboxShader/skyboxVS.hlsl",
		L"shaders/skyboxShader/skyboxPS.hlsl", nullptr, nullptr, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	auto skyboxTexture = Engine::TextureManager::Init()->LoadFromFile("skybox", L"Textures\\mountains.dds");

	skybox.SetShader(skyboxShader);
	skybox.SetTexture(skyboxTexture);
	skybox.BindCamera(camera.get());
	Engine::Renderer::Init()->setSkyBox(std::make_shared<Engine::SkyBox>(skybox));
}

void D3DApplication::InitPostProcess()
{
	auto postshader = Engine::ShaderManager::CompileAndCreateShader("PostProcess", L"shaders/PostProcess/PostProcessVS.hlsl", L"shaders/PostProcess/PostProcessPS.hlsl",
		nullptr, nullptr, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto FXAA = Engine::ShaderManager::CompileAndCreateShader("FXAA", L"Shaders\\fullScreenVS.hlsl",
		L"Shaders\\PostProcess\\fxaa.hlsl", nullptr, nullptr);

	Engine::PostProcess::Init()->SetLightToColorShader(postshader);
	Engine::PostProcess::Init()->SetFXAAShader(FXAA);
}
