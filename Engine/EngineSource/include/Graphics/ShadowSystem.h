#pragma once
#include "Graphics/D3D.h"
#include "Graphics/MeshSystem.h"
#include "Graphics/LightSystem.h"
#include "Render/Camera.h"
#include "Math/math.h"
#include <vector>
#include <mutex>
#undef min

namespace Engine
{
	struct vec3;

	class ShadowSystem
	{
	public:
		struct ShadowRenderGroupInfo
		{
			MeshSystem::RenderGroups renderFlag;
			std::shared_ptr<shader> usedShader;
		};
	public:
		ShadowSystem(const ShadowSystem& other) = delete;
		void operator=(const ShadowSystem&) = delete;

		static ShadowSystem* Init();
		static void Deinit();

		void SetShadowShaders(std::shared_ptr<shader> pointLight, std::shared_ptr<shader> spotLight, std::shared_ptr<shader> directionalLight);

		void SetShadowTextureResolution(UINT resolution);
		UINT GetShadowTextureResolution();

		void SetProjectionInfo(float nearPlane, float farPlane);
		float GetProjectionFarPlane() { return m_ProjectionInfo.farPlane; }

		void PrecomputeDirectionalProjections(const std::vector<DirectionalLight>& lights, const Camera* camera);
		void PrecomputeSpotProjections(const std::vector<SpotLight>& lights);


		void RenderPointLightShadowMaps(const std::vector<vec3>& lightPositions, const std::vector<ShadowRenderGroupInfo>& renderGroupsInfo);

		void RenderSpotLightShadowMaps(const std::vector<ShadowRenderGroupInfo>& renderGroupsInfo);

		void RenderDirectLightShadowMaps(const std::vector<ShadowRenderGroupInfo>& renderGroupsInfo);

		void BindShadowTextures(UINT pointLightTexture, UINT spotLightTexture, UINT directionaLightTexture);
		void BindShadowBuffers(UINT spotLightsBufferSlot, UINT directionalLightsBufferSlot);

	private:
		void groupToRenderIn(MeshSystem::RenderGroups renderGroupFlags, std::shared_ptr<shader> usedShader);

		ShadowSystem();
		~ShadowSystem() = default;

		void createPointLightShadowMaps(size_t amount);
		void Create2DShadowMaps(size_t amount, std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>>& dsvs, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv);

	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srvPointLigts;
		std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_plDSVS;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srvSpotLigts;
		std::vector <Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_slDSVS;
		std::vector<mat4> m_slViewProjections;
		ConstBuffer<mat4> m_slConstBuff;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srvDirLigts;
		std::vector <Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_dlDSVS;
		std::vector<mat4> m_dlViewProjections;
		ConstBuffer<mat4> m_dlConstBuff;

		D3D11_VIEWPORT m_viewport;
		UINT m_shadowResolution = 2048;

		struct ProjectionInfo
		{
			float nearPlane = 0.01f;
			float farPlane = 100.0f;
		} m_ProjectionInfo;


		std::shared_ptr<shader> m_plShader;
		std::shared_ptr<shader> m_slShader;
		std::shared_ptr<shader> m_dlShader;
	private:
		static ShadowSystem* m_instance;
		static std::mutex m_mutex;
	};
}

