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


	auto NormalVisColor = Engine::ShaderManager::CompileAndCreateShader("NormalVisColor", L"Shaders\\normalColor\\VertexShader.hlsl",
		L"Shaders\\normalColor\\PixelShader.hlsl", nullptr, nullptr);

	auto inputLayout = Engine::ShaderManager::CreateInputLayout("Default", NormalVisColor->vertexBlob.Get(), ied, 9u);

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

	auto ms = Engine::MeshSystem::Init();

	ms->normVisGroup.addShader(NormalVisLines);
	ms->normVisGroup.addShader(NormalVisColor);
	ms->hologramGroup.addShader(HologramGroup);
	ms->hologramGroup.addShader(NormalVisLines);

	ms->textureGroup.addShader(textureMap);
	ms->textureGroup.addShader(NormalVisLines);

	ms->opaqueGroup.addShader(opaqueShader);
	ms->opaqueGroup.addShader(NormalVisLines);
}

D3DApplication::D3DApplication(int windowWidth, int windowHeight, WinProc windowEvent) :
	pWindow(new Engine::Window(windowWidth, windowHeight, windowEvent))
{
	camera.reset(new Engine::Camera(45.0f, 0.1f, 100.0f));
	
	camera->calculateProjectionMatrix(windowWidth, windowHeight);
	camera->calculateRayDirections();
	InitMeshSystem();
	
	auto TM = Engine::TextureManager::Init();
	auto crateFirst = TM->LoadFromFile("crate", L"Textures\\crate.dds");
	auto crateSecond = TM->LoadFromFile("metalCrate", L"Textures\\MetalCrate.dds");


	Engine::MeshSystem::Material knightMat = { Engine::vec3(1.0,0.0f,1.0f), 0.0f,Engine::vec3(1.0,1.0f,0.0f), 0.0f };

	auto changepos = [](Engine::TransformSystem::transforms& inst, const Engine::vec3& pos) {
		for (size_t i = 0; i < 3; i++)
		{
			inst.modelToWold[3][i] = pos[i];
		}
		};

	auto changescale = [](Engine::TransformSystem::transforms& inst,int axis, const float scale) {
		for (size_t i = 0; i < 3; i++)
		{
			inst.modelToWold[axis][i] *= scale;
		}
		};
	std::vector<Engine::MeshSystem::TextureMaterial> samuraiTextures;
	samuraiTextures.push_back(Engine::MeshSystem::TextureMaterial{ TM->LoadFromFile("samurai_sword", L"Textures\\Samurai\\Sword_BaseColor.dds") });
	samuraiTextures.push_back(Engine::MeshSystem::TextureMaterial{ TM->LoadFromFile("samurai_head", L"Textures\\Samurai\\Head_BaseColor.dds") });
	samuraiTextures.push_back(Engine::MeshSystem::TextureMaterial{ TM->LoadFromFile("samurai_eyes", L"Textures\\Samurai\\Eyes_BaseColor.dds") });
	samuraiTextures.push_back(Engine::MeshSystem::TextureMaterial{ TM->LoadFromFile("samurai_helmet", L"Textures\\Samurai\\Helmet_BaseColor.dds") });
	samuraiTextures.push_back(Engine::MeshSystem::TextureMaterial{ TM->LoadFromFile("samurai_decor", L"Textures\\Samurai\\Decor_BaseColor.dds") });
	samuraiTextures.push_back(Engine::MeshSystem::TextureMaterial{ TM->LoadFromFile("samurai_pants", L"Textures\\Samurai\\Pants_BaseColor.dds") });
	samuraiTextures.push_back(Engine::MeshSystem::TextureMaterial{ TM->LoadFromFile("samurai_hands", L"Textures\\Samurai\\Hands_BaseColor.dds") });
	samuraiTextures.push_back(Engine::MeshSystem::TextureMaterial{ TM->LoadFromFile("samurai_torso", L"Textures\\Samurai\\Torso_BaseColor.dds") });

	auto model = Engine::ModelManager::GetInstance()->loadModel("Models\\Samurai.fbx");
	Engine::TransformSystem::transforms inst = { Engine::transformMatrix(Engine::vec3(0.0f, -1.0f, 0.0f), Engine::vec3(0.0f, 0.0f, 1.0f), Engine::vec3(1.0f, 0.0f, 0.0f), Engine::vec3(0.0f, 1.0f, 0.0f)) };
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, samuraiTextures, inst);

	Engine::PointLight pointLight(Engine::vec3(1.0f, 1.0f, 1.0f), Engine::vec3(0.0f), 10.0f);
	Engine::DirectionalLight directionalLight(Engine::vec3(0.707f, -0.707f, 0.0f), Engine::vec3(1.0f), 25.0f);
	Engine::TransformSystem::transforms pointTransfom = { Engine::transformMatrix(Engine::vec3(0.0f, 1.0f, 0.0f), Engine::vec3(0.0f, 0.0f, 1.0f), Engine::vec3(1.0f, 0.0f, 0.0f), Engine::vec3(0.0f, 1.0f, 0.0f)) };
	uint32_t id = Engine::TransformSystem::Init()->AddModelTransform(pointTransfom, 1.0f);
	pointLight.bindedObjectId = id;
	Engine::LightSystem::Init()->AddPointLight(pointLight);
	Engine::LightSystem::Init()->AddDirectionalLight(directionalLight);
	Engine::LightSystem::Init()->UpdateLightsBuffer();

	//knightMat = { Engine::vec3(0.0,1.0f,1.0f), 0.0f,Engine::vec3(1.0,0.0f,0.0f), 0.0f };
	changepos(inst, Engine::vec3(1.0f, -1.0f, 0.0f));
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, samuraiTextures, inst);

	model = Engine::ModelManager::GetInstance()->loadModel("Models\\cube.obj");
	//changepos(inst, Engine::vec3(1.0f, 1.0f, 5.0f));
	//Engine::MeshSystem::Init()->normVisGroup.addModel(model, knightMat, inst);

	//changepos(inst, Engine::vec3(3.0f, -1.0f, -2.0f));
	//Engine::MeshSystem::Init()->normVisGroup.addModel(model, knightMat, inst);

	Engine::MeshSystem::TextureMaterial crateMaterial;
	crateMaterial = Engine::MeshSystem::TextureMaterial{ crateFirst };
	changepos(inst, Engine::vec3(1.0f, -4.0f, 2.0f));
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, crateMaterial, inst);

	//auto rotX = Engine::mat4::rotateX(3.14f * (-45.0f) / 360.0f);

	//changepos(inst, Engine::vec3(-4.0f, 0.0f, 1.0f));
	//Engine::MeshSystem::Init()->hologramGroup.addModel(model, knightMat, Engine::TransformSystem::transforms{ inst.modelToWold * rotX });
	
	auto rotZ = Engine::mat4::rotateZ(3.14f * (-45.0f) / 360.0f);
	changescale(inst,0, 5);
	crateMaterial.texture = crateSecond;
	changepos(inst, Engine::vec3(-10.0f, -4.0f, 2.0f));
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, crateMaterial, Engine::TransformSystem::transforms{ inst.modelToWold * rotZ });

	
	//changescale(inst, 0, 0.2f);
	//changepos(inst, Engine::vec3(2.0f, -2.0f, 4.0f));
	//Engine::MeshSystem::Init()->hologramGroup.addModel(model, knightMat, Engine::TransformSystem::transforms{ inst.modelToWold * rotZ });


	Engine::MeshSystem::Init()->updateInstanceBuffers();

	auto skyboxShader = Engine::ShaderManager::CompileAndCreateShader("skybox", L"shaders/skyboxShader/skyboxVS.hlsl", 
		L"shaders/skyboxShader/skyboxPS.hlsl", nullptr, nullptr, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	auto skyboxTexture = Engine::TextureManager::Init()->LoadFromFile("skybox", L"Textures\\night_street.dds");

	auto postshader = Engine::ShaderManager::CompileAndCreateShader("PostProcess", L"shaders/PostProcess/PostProcessVS.hlsl", L"shaders/PostProcess/PostProcessPS.hlsl",
		nullptr, nullptr, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Engine::PostProcess::Init()->SetLightToColorShader(postshader);

	skybox.SetShader(skyboxShader);
	skybox.SetTexture(skyboxTexture);
	skybox.BindCamera(camera.get());
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
