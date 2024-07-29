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
#include <assert.h>

#ifdef UNICODE
typedef std::wostringstream tstringstream;
#else
typedef std::ostringstream tstringstream;
#endif

Engine::vec2 previousMousePosition;
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
	{"TOWORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
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
	{"EMISSION", 0, DXGI_FORMAT_R32G32B32_FLOAT , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	auto emissiveShader = Engine::ShaderManager::CompileAndCreateShader("EmmisiveShader", L"Shaders\\emissive\\emissiveVS.hlsl",
		L"Shaders\\emissive\\emissivePS.hlsl", nullptr, nullptr);

	auto NormalVisColor = Engine::ShaderManager::CompileAndCreateShader("NormalVisColor", L"Shaders\\normalColor\\VertexShader.hlsl",
		L"Shaders\\normalColor\\PixelShader.hlsl", nullptr, nullptr);

	auto inputLayout = Engine::ShaderManager::CreateInputLayout("Default", NormalVisColor->vertexBlob.Get(), ied, 9u);
	auto secondInputLayout = Engine::ShaderManager::CreateInputLayout("Second", emissiveShader->vertexBlob.Get(), secondIed, 10u);

	auto textureMap = Engine::ShaderManager::CompileAndCreateShader("texture", L"Shaders\\crateTextMap\\CrateVS.hlsl",
		L"Shaders\\crateTextMap\\CratePS.hlsl", nullptr, nullptr);

	D3D_SHADER_MACRO shaders[] = { "MAX_DIRECTIONAL_LIGHTS", "1",
		"MAX_POINT_LIGHTS", "10",
		"MAX_SPOT_LIGHTS","10",
		NULL,NULL};

	auto opaqueShader = Engine::ShaderManager::CompileAndCreateShader("opaque", L"Shaders\\opaqueShader\\opaqueVS.hlsl",
		L"Shaders\\opaqueShader\\opaquePS.hlsl", nullptr, shaders);

	auto NormalVisLines = Engine::ShaderManager::CompileAndCreateShader("NormalVisLines", L"Shaders\\normalLines\\VertexShader.hlsl",
		L"Shaders\\normalLines\\PixelShader.hlsl", L"Shaders\\normalLines\\HullShader.hlsl", L"Shaders\\normalLines\\DomainShader.hlsl",
		L"Shaders\\normalLines\\GSnormal.hlsl", nullptr, nullptr, D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	NormalVisLines->DisableShader();
	if (!NormalVisColor)
		throw std::runtime_error("Failed to compile and create shader!");

	auto HologramGroup = Engine::ShaderManager::CompileAndCreateShader("HologramGroup", L"Shaders\\Hologram\\Hologram.shader",
		L"Shaders\\Hologram\\Hologram.shader", L"Shaders\\Hologram\\HullShader.hlsl", L"Shaders\\Hologram\\DomainShader.hlsl", L"Shaders\\Hologram\\GSHologram.hlsl",
		nullptr, nullptr, D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST, "vsMain", "psMain");

	if (!HologramGroup)
		throw std::runtime_error("Failed to compile and create shader!");

	NormalVisColor->BindInputLyout(inputLayout);
	NormalVisLines->BindInputLyout(inputLayout);
	HologramGroup->BindInputLyout(inputLayout);
	textureMap->BindInputLyout(inputLayout);
	opaqueShader->BindInputLyout(inputLayout);
	emissiveShader->BindInputLyout(secondInputLayout);

	auto ms = Engine::MeshSystem::Init();

	ms->normVisGroup.addShader(NormalVisLines);
	ms->normVisGroup.addShader(NormalVisColor);
	ms->hologramGroup.addShader(HologramGroup);
	ms->hologramGroup.addShader(NormalVisLines);

	ms->textureGroup.addShader(textureMap);
	ms->textureGroup.addShader(NormalVisLines);

	ms->opaqueGroup.addShader(opaqueShader);
	ms->opaqueGroup.addShader(NormalVisLines);

	ms->emmisiveGroup.addShader(emissiveShader);
}

D3DApplication::D3DApplication(int windowWidth, int windowHeight, WinProc windowEvent) :
	pWindow(new Engine::Window(windowWidth, windowHeight, windowEvent)) {
	InitCamera(windowWidth, windowHeight);
	InitMeshSystem();
	InitSamuraiModel();
	InitLights();
	InitCrateModel();
	InitFloor();
	InitSkybox();
	InitPostProcess();
	Engine::MeshSystem::Init()->updateInstanceBuffers();
}


bool D3DApplication::isClosed()
{
	return pWindow->isClosed();
}

void D3DApplication::Update(float deltaTime)
{
	UpdateInput(deltaTime);
	if (pWindow->wasWindowResized())
	{
		camera->calculateProjectionMatrix(pWindow->getWindowWidth(), pWindow->getWindowHeight());
	}

	Engine::D3D* d3d = Engine::D3D::GetInstance();
	Engine::Renderer* renderer = Engine::Renderer::GetInstance();
	renderer->updatePerFrameCB(deltaTime, (FLOAT)pWindow->getWindowWidth(), (FLOAT)pWindow->getWindowHeight());

	Engine::TextureManager::Init()->BindSamplers();

	renderer->Render(camera.get());

	skybox.BindSkyBox(2u);
	skybox.Draw();
		
	renderer->PostProcess();

	pWindow->flush();
}

void D3DApplication::UpdateInput(float deltaTime)
{
	Engine::vec2 mousePosition = Input::getMousePosition();

	Engine::vec3 cameraMoveDirection = (0.0f, 0.0f, 0.0f);
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

	if (Input::keyPresseed(Input::KeyboardButtons::ONE))
		Engine::TextureManager::Init()->BindSampleByFilter(D3D11_FILTER_MIN_MAG_MIP_POINT, 3u);
	else if (Input::keyPresseed(Input::KeyboardButtons::TWO))
		Engine::TextureManager::Init()->BindSampleByFilter(D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, 3u);
	else if (Input::keyPresseed(Input::KeyboardButtons::THREE))
		Engine::TextureManager::Init()->BindSampleByFilter(D3D11_FILTER_ANISOTROPIC, 3u);

	if (Input::keyPresseed(Input::KeyboardButtons::N))
	{
		auto visShader = Engine::ShaderManager::GetShader("NormalVisLines");

		if (visShader)
			visShader->isEnabled = !visShader->isEnabled;
	}


	int scrolls = Input::scrollAmount();
	if (scrolls > 0)
		cameraSpeed += cameraSpeed * 0.1f * scrolls;
	else if (scrolls < 0)
		cameraSpeed += cameraSpeed * 0.1f * scrolls;




	camera->moveCamera(cameraMoveDirection * cameraSpeed);

	bool cameraRotated = false;
	Engine::vec2 delta;
	if (Input::mouseIsDown(Input::MouseButtons::LEFT))
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

	if (Input::mouseWasPressed(Input::MouseButtons::RIGHT))
	{
		Engine::vec2 screenCoord(mousePosition.x, pWindow->getWindowHeight() - mousePosition.y);
		Engine::ray r;
		screenCoord.x = (screenCoord.x / pWindow->getWindowWidth() - 0.5f) * 2.0f;
		screenCoord.y = (screenCoord.y / pWindow->getWindowHeight() - 0.5f) * 2.0f;
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

	if (cameraMoveDirection != Engine::vec3(0.0f) || cameraRotated)
	{
		camera->calculateViewMatrix();
		camera->setRight(Engine::cross(camera->getUp(), camera->getForward()));
		camera->calculateRayDirections();
	}

	if (dragger)
	{
		Engine::vec2 screenCoord(mousePosition.x, pWindow->getWindowHeight() - mousePosition.y);
		Engine::ray r;
		screenCoord.x = (screenCoord.x / pWindow->getWindowWidth() - 0.5f) * 2.0f;
		screenCoord.y = (screenCoord.y / pWindow->getWindowHeight() - 0.5f) * 2.0f;
		r.origin = camera->getPosition();
		r.direction = camera->calculateRayDirection(screenCoord).normalized();

		dragger->drag(r);
	}
}

D3DApplication::~D3DApplication()
{
}

void D3DApplication::InitCamera(int windowWidth, int windowHeight)
{
	camera.reset(new Engine::Camera(45.0f, 0.1f, 100.0f));
	camera->calculateProjectionMatrix(windowWidth, windowHeight);
	camera->calculateRayDirections();
}

void D3DApplication::InitSamuraiModel()
{
	auto TM = Engine::TextureManager::Init();

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
		  TM->LoadFromFile("samurai_eyes_normal", L"Textures\\Samurai\\Eyes_Normal.dds") },
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

	auto model = Engine::ModelManager::GetInstance()->loadModel("Models\\Samurai.fbx");
	Engine::TransformSystem::transforms inst = {
		Engine::transformMatrix(Engine::vec3(0.0f, -1.0f, 0.0f), Engine::vec3(0.0f, 0.0f, 1.0f), Engine::vec3(1.0f, 0.0f, 0.0f), Engine::vec3(0.0f, 1.0f, 0.0f)) };
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, samuraiTextures, inst);

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

	float radiance = Engine::Light::radianceFromIrradiance(1.35f, 0.5f, 1.0f);
	Engine::PointLight pointLight(Engine::vec3(1.0f, 1.0f, 1.0f) * radiance, Engine::vec3(0.0f), 0.5f);
	Engine::PointLight pointLight3(Engine::vec3(0.0f, 1.0f, 0.0f) * radiance, Engine::vec3(0.0f), 0.5f);

	radiance = Engine::Light::radianceFromIrradiance(1.0f, 1.0f, 5.0f);
	Engine::PointLight pointLight2(Engine::vec3(1.5f, 0.0f, 0.9f) * radiance, Engine::vec3(0.0f), 1.0f);
	

	Engine::SpotLight spotLight(Engine::vec3(100.0f), Engine::vec3(0.0f, 0.0f, 0.0f), Engine::vec3(.0f, .0f, 1.0f), 0.5 / 2.0f, 1.0f);
	spotLight.bindedObjectId = camera->getCameraTransformId();
	Engine::DirectionalLight directionalLight(Engine::vec3(0.707f, -0.707f, 0.0f), Engine::vec3(0.84f * 10.0f, 0.86264f * 10.0f, 0.89019f * 10.0f), 0.15f);

	Engine::ModelManager::GetInstance()->initUnitSphere();
	auto model = Engine::ModelManager::GetInstance()->GetModel("UNIT_SPHERE");

	Engine::TransformSystem::transforms inst = {
		Engine::transformMatrix(Engine::vec3(0.0f, -1.0f, 0.0f), Engine::vec3(0.0f, 0.0f, 1.0f), Engine::vec3(1.0f, 0.0f, 0.0f), Engine::vec3(0.0f, 1.0f, 0.0f)) };

	auto emmisiveSphereInstance = inst;

	changepos(emmisiveSphereInstance, Engine::vec3(-5.0f, 0.0f, 2.0f));
	pointLight2.bindedObjectId = Engine::MeshSystem::Init()->emmisiveGroup.addModel(model, Materials::EmmisiveMaterial{}, emmisiveSphereInstance, Engine::MeshSystem::EmmisiveInstance{ Engine::vec3(15.0f, 0.0f, 9.0f) });

	changescale(emmisiveSphereInstance, 0, 0.5f); changescale(emmisiveSphereInstance, 1, 0.5f); changescale(emmisiveSphereInstance, 2, 0.5f);
	changepos(emmisiveSphereInstance, Engine::vec3(2.0f, -1.0f, 0.0f));
	pointLight.bindedObjectId = Engine::MeshSystem::Init()->emmisiveGroup.addModel(model, Materials::EmmisiveMaterial{}, emmisiveSphereInstance, Engine::MeshSystem::EmmisiveInstance{ Engine::vec3(10.0f, 10.0f, 10.0f) });

	changepos(emmisiveSphereInstance, Engine::vec3(2.0f, 2.0f, 0.0f));
	pointLight3.bindedObjectId = Engine::MeshSystem::Init()->emmisiveGroup.addModel(model, Materials::EmmisiveMaterial{}, emmisiveSphereInstance, Engine::MeshSystem::EmmisiveInstance{ Engine::vec3(0.0f, 10.0f, 0.0f) });

	Engine::LightSystem::Init()->AddFlashLight(spotLight, TM->LoadFromFile("flashlight", L"Textures\\flashlightMask.dds"));
	Engine::LightSystem::Init()->AddPointLight(pointLight);
	Engine::LightSystem::Init()->AddPointLight(pointLight2);
	Engine::LightSystem::Init()->AddPointLight(pointLight3);
	Engine::LightSystem::Init()->AddDirectionalLight(directionalLight);
	Engine::LightSystem::Init()->UpdateLightsBuffer();
}

void D3DApplication::InitCrateModel()
{
	auto model = Engine::ModelManager::GetInstance()->loadModel("Models\\cube.obj", true);

	auto TM = Engine::TextureManager::Init();
	auto crateFirst = TM->LoadFromFile("crate", L"Textures\\RedCore\\albedo.dds");
	auto crateMetallic = TM->LoadFromFile("crateMetallic", L"Textures\\RedCore\\metallic.dds");
	auto crateRoughness = TM->LoadFromFile("crateRoughness", L"Textures\\RedCore\\roughness.dds");
	auto crateNormal = TM->LoadFromFile("crateNormal", L"Textures\\RedCore\\normal.dds");

	Materials::OpaqueTextureMaterial crateMaterial = { crateFirst, crateRoughness, crateMetallic, crateNormal };

	Engine::TransformSystem::transforms inst = {
		Engine::transformMatrix(Engine::vec3(0.0f, -1.0f, 0.0f), Engine::vec3(0.0f, 0.0f, 1.0f), Engine::vec3(1.0f, 0.0f, 0.0f), Engine::vec3(0.0f, 1.0f, 0.0f)) };

	changepos(inst, Engine::vec3(1.0f, -1.0f, 4.0f));
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, crateMaterial, inst);

	auto rotZ = Engine::mat4::rotateZ(3.14f * (-45.0f) / 360.0f);
	changescale(inst, 0, 5);
	changepos(inst, Engine::vec3(-10.0f, 4.0f, 2.0f));
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

void D3DApplication::InitSkybox()
{
	auto skyboxShader = Engine::ShaderManager::CompileAndCreateShader("skybox", L"shaders/skyboxShader/skyboxVS.hlsl",
		L"shaders/skyboxShader/skyboxPS.hlsl", nullptr, nullptr, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	auto skyboxTexture = Engine::TextureManager::Init()->LoadFromFile("skybox", L"Textures\\night_street.dds");

	skybox.SetShader(skyboxShader);
	skybox.SetTexture(skyboxTexture);
	skybox.BindCamera(camera.get());
}

void D3DApplication::InitPostProcess()
{
	auto postshader = Engine::ShaderManager::CompileAndCreateShader("PostProcess", L"shaders/PostProcess/PostProcessVS.hlsl", L"shaders/PostProcess/PostProcessPS.hlsl",
		nullptr, nullptr, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Engine::PostProcess::Init()->SetLightToColorShader(postshader);
}
