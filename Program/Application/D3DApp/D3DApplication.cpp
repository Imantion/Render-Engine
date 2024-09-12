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
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Graphics/ShadowSystem.h"
#include "Utils/ISelected.h"
#include <assert.h>

#ifdef UNICODE
typedef std::wostringstream tstringstream;
#else
typedef std::ostringstream tstringstream;
#endif

Engine::vec2 previousMousePosition;
static std::vector<Materials::DissolutionMaterial> samuraiDisolutionMaterial;
static Materials::DissolutionMaterial cubeDisolutionMaterial;
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
	};

	auto emissiveShader = Engine::ShaderManager::CompileAndCreateShader("EmmisiveShader", L"Shaders\\emissive\\emissiveVS.hlsl",
		L"Shaders\\emissive\\emissivePS.hlsl", nullptr, nullptr);

	auto NormalVisColor = Engine::ShaderManager::CompileAndCreateShader("NormalVisColor", L"Shaders\\normalColor\\VertexShader.hlsl",
		L"Shaders\\normalColor\\PixelShader.hlsl", nullptr, nullptr);

	auto dissolutionShader = Engine::ShaderManager::CompileAndCreateShader("DissolutionShader", L"Shaders\\opaqueShader\\dissolutionVS.hlsl", L"Shaders\\opaqueShader\\dissolutionPS.hlsl",
		nullptr, nullptr);

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


	auto shadowShader = Engine::ShaderManager::CompileAndCreateShader("PLshadow", L"Shaders\\Shadow\\PointLightShadowVS.hlsl", L"Shaders\\Shadow\\PointLightShadowPS.hlsl",
		nullptr, nullptr, L"Shaders\\Shadow\\PointLightShadowGS.hlsl", nullptr, nullptr);

	auto shadowShader2 = Engine::ShaderManager::CompileAndCreateShader("shadow", L"Shaders\\Shadow\\ShadowVS.hlsl", L"Shaders\\Shadow\\ShadowPS.hlsl", nullptr, nullptr);

	auto shadowShader3 = Engine::ShaderManager::CompileAndCreateShader("DissPLshadow", L"Shaders\\Shadow\\Dissolution\\PointLightShadowVS.hlsl", L"Shaders\\Shadow\\Dissolution\\PointLightShadowPS.hlsl",
		nullptr, nullptr, L"Shaders\\Shadow\\Dissolution\\PointLightShadowGS.hlsl", nullptr, nullptr);

	auto shadowShader4 = Engine::ShaderManager::CompileAndCreateShader("DissShadow", L"Shaders\\Shadow\\Dissolution\\ShadowVS.hlsl", L"Shaders\\Shadow\\Dissolution\\ShadowPS.hlsl", nullptr, nullptr);

	NormalVisLines->DisableShader();
	if (!NormalVisColor)
		throw std::runtime_error("Failed to compile and create shader!");

	auto HologramGroup = Engine::ShaderManager::CompileAndCreateShader("HologramGroup", L"Shaders\\Hologram\\Hologram.shader",
		L"Shaders\\Hologram\\Hologram.shader", L"Shaders\\Hologram\\HullShader.hlsl", L"Shaders\\Hologram\\DomainShader.hlsl", L"Shaders\\Hologram\\GSHologram.hlsl",
		nullptr, nullptr, D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST, "vsMain", "psMain");

	if (!HologramGroup)
		throw std::runtime_error("Failed to compile and create shader!");

	auto inputLayout = Engine::ShaderManager::CreateInputLayout("Default", opaqueShader->vertexBlob.Get(), ied, sizeof(ied) / sizeof(D3D11_INPUT_ELEMENT_DESC));
	auto secondInputLayout = Engine::ShaderManager::CreateInputLayout("Second", emissiveShader->vertexBlob.Get(), secondIed, sizeof(secondIed) / sizeof(D3D11_INPUT_ELEMENT_DESC));
	auto thirdLayout = Engine::ShaderManager::CreateInputLayout("Third", NormalVisLines->vertexBlob.Get(), secondIed, sizeof(normalIED) / sizeof(D3D11_INPUT_ELEMENT_DESC));
	auto fourthLayout = Engine::ShaderManager::CreateInputLayout("Fourth", dissolutionShader->vertexBlob.Get(), thirdIed, sizeof(thirdIed) / sizeof(D3D11_INPUT_ELEMENT_DESC));

	NormalVisColor->BindInputLyout(thirdLayout);
	NormalVisLines->BindInputLyout(thirdLayout);
	HologramGroup->BindInputLyout(thirdLayout);
	textureMap->BindInputLyout(thirdLayout);
	opaqueShader->BindInputLyout(inputLayout);
	emissiveShader->BindInputLyout(secondInputLayout);
	shadowShader->BindInputLyout(thirdLayout);
	shadowShader2->BindInputLyout(thirdLayout);
	shadowShader3->BindInputLyout(fourthLayout);
	shadowShader4->BindInputLyout(fourthLayout);
	dissolutionShader->BindInputLyout(fourthLayout);

	Engine::ShadowSystem::Init()->SetShadowShaders(shadowShader, shadowShader2, shadowShader2);

	auto ms = Engine::MeshSystem::Init();

	ms->normVisGroup.addShader(NormalVisLines);
	ms->normVisGroup.addShader(NormalVisColor);
	ms->hologramGroup.addShader(HologramGroup);
	ms->hologramGroup.addShader(NormalVisLines);

	ms->opaqueGroup.addShader(opaqueShader);
	ms->opaqueGroup.addShader(NormalVisLines);

	ms->emmisiveGroup.addShader(emissiveShader);

	ms->dissolutionGroup.addShader(dissolutionShader);
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
	ImGui_ImplWin32_Init(pWindow->getHWND());	
	Engine::MeshSystem::Init()->updateInstanceBuffers();
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
	ShadingGroupSwap();

	Engine::TextureManager::Init()->BindSamplers();
	Engine::ParticleSystem::Init()->Update(deltaTime);

	Engine::Renderer* renderer = Engine::Renderer::GetInstance();
	renderer->updatePerFrameCB(deltaTime, (FLOAT)pWindow->getWindowWidth(), (FLOAT)pWindow->getWindowHeight(), camera->getNearClip(), camera->getFarClip());
	renderer->Render(camera.get());
		
	renderer->PostProcess();

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

	if (Input::mouseWasPressed(Input::MouseButtons::RIGHT) && objectInteractions == Drag)
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

	if (Input::mouseWasPressed(Input::MouseButtons::RIGHT) && objectInteractions == Select)
	{
	
		Engine::vec2 screenCoord(mousePosition.x, pWindow->getWindowHeight() - mousePosition.y);
		Engine::ray r;
		screenCoord.x = (screenCoord.x / pWindow->getWindowWidth() - 0.5f) * 2.0f;
		screenCoord.y = (screenCoord.y / pWindow->getWindowHeight() - 0.5f) * 2.0f;
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

	Engine::ModelManager::GetInstance()->initUnitSphere();
	auto model = Engine::ModelManager::GetInstance()->GetModel("UNIT_SPHERE");

	Engine::LightSystem::Init()->AddPointLight(Engine::vec3(1.5f, 0.0f, 0.9f), 1.0f, 3.0f, Engine::vec3(-5.0f, 0.0f, 2.0f), model);
	Engine::LightSystem::Init()->AddPointLight(Engine::vec3(2.35f), 0.5f, 1.84f, Engine::vec3(2.0f, -1.0f, 0.0f), model);
	Engine::LightSystem::Init()->AddPointLight(Engine::vec3(0.0f, 4.0f, 0.0f), 0.5f, 2.25f, Engine::vec3(2.0f, 2.0f, 0.0f), model);
	Engine::LightSystem::Init()->AddPointLight(Engine::vec3(0.1f), 0.05f, 1.4f, Engine::vec3(0.0f, 0.0f, -0.5f), model);

	Engine::SpotLight spotLight(Engine::vec3(1.0f), 1.0f, 11.18f, Engine::vec3(0.0f, 0.0f, 0.0f), Engine::vec3(.0f, .0f, 1.0f), 0.5 / 2.0f);
	spotLight.bindedObjectId = camera->getCameraTransformId();
	Engine::LightSystem::Init()->AddFlashLight(spotLight, TM->LoadFromFile("flashlight", L"Textures\\flashlightMask.dds"));

	Engine::DirectionalLight directionalLight(Engine::vec3(-0.3205475307f, -0.595605361f, -0.10348193f).normalized(), Engine::vec3(0.84f * 7.5f, 0.86264f * 7.5f, 0.89019f * 7.5f), 0.15f);
	Engine::LightSystem::Init()->AddDirectionalLight(directionalLight);

	Engine::TransformSystem::transforms bombo = {
		Engine::transformMatrix(Engine::vec3(0.0f, 8.0f, 0.0f), Engine::vec3(0.0f, 0.0f, 0.1f), Engine::vec3(0.1f, 0.0f, 0.0f), Engine::vec3(0.0f, 0.1f, 0.0f)) };

	auto id = Engine::MeshSystem::Init()->emmisiveGroup.addModel(model, Materials::EmmisiveMaterial{}, bombo, Instances::EmmisiveInstance{ Engine::vec3(1.0f) });
	Engine::Emitter emitter(Engine::vec3(0.0f), id, 50, 0.1f, Engine::vec3(1.0f));
	Engine::ParticleSystem::Init()->addSmokeEmitter(emitter);
	changepos(bombo, Engine::vec3(8.0f, -2.0f, 0.0f));
	id = Engine::MeshSystem::Init()->emmisiveGroup.addModel(model, Materials::EmmisiveMaterial{}, bombo, Instances::EmmisiveInstance{ Engine::vec3(1.0f,0.0f,1.0f) });
	Engine::Emitter emitter2(Engine::vec3(0.0f), id, 50, 0.1f, Engine::vec3(1.0f,0.0f,1.0f));
	emitter2.adjustParameters(Engine::vec3(0.15f, 0.4f, 0.15f), Engine::vec2(0.5f), 50, 10.0f);
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
	changepos(inst, Engine::vec3(-3.0f, -1.0f, 2.0f));
	Engine::MeshSystem::Init()->opaqueGroup.addModel(model, goldenCube, inst);

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

	Engine::PostProcess::Init()->SetLightToColorShader(postshader);
}
