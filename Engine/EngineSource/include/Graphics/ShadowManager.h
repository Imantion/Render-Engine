#pragma once
#include "Graphics/D3D.h"
#include "MeshSystem.h"
#include "LightSystem.h"
#include <vector>
#include <mutex>

namespace Engine
{
	struct vec3;

	class ShadowManager
	{
	public:
		ShadowManager(const ShadowManager& other) = delete;
		void operator=(const ShadowManager&) = delete;

		static ShadowManager* Init();
		static void Deinit();

		void SetShadowShaders(std::shared_ptr<shader> pointLight, std::shared_ptr<shader> spotLight, std::shared_ptr<shader> directionalLight);

		void SetShadowTextureResolution(UINT resolution);
		UINT GetShadowTextureResolution();

		void SetProjectionInfo(float nearPlane, float farPlane);

		template <typename I, typename M>
		void RenderPointLightShadowMaps(const std::vector<vec3>& lightPositions, OpaqueInstances<I, M>& renderGroup);

		template <typename I, typename M>
		void RenderSpotLightShadowMaps(const std::vector<SpotLight>& lights, OpaqueInstances<I, M>& renderGroup);

		void BindSRV(UINT slot) { D3D::GetInstance()->GetContext()->PSSetShaderResources(slot, 1u, m_srvPointLigts.GetAddressOf()); }

	private:
		ShadowManager();
		~ShadowManager() = default;

		void createPointLightShadowMaps(size_t amount);
		void createSpotLightShadowMaps(size_t amount);

	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srvPointLigts;
		std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_plDSVS;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srvSpotLigts;
		std::vector <Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_slDSVS;

		D3D11_VIEWPORT m_viewport;
		UINT m_shadowResolution = 1024;

		struct ProjectionInfo
		{
			float nearPlane = 0.01f;
			float farPlane = 100.0f;
		} m_ProjectionInfo;


		std::shared_ptr<shader> m_plShader;
		std::shared_ptr<shader> m_slShader;
		std::shared_ptr<shader> m_dlShader;
	private:
		static ShadowManager* m_instance;
		static std::mutex m_mutex;
	};

	template<typename I, typename M>
	inline void ShadowManager::RenderPointLightShadowMaps(const std::vector<vec3>& lightPositions, OpaqueInstances<I, M>& renderGroup)
	{
		if (m_plDSVS.size() != lightPositions.size())
			createPointLightShadowMaps(lightPositions.size());

		auto context = D3D::GetInstance()->GetContext();

		mat4 projection = projectionMatrix((float)M_PI_2, m_ProjectionInfo.nearPlane, m_ProjectionInfo.farPlane, 1.0f);

		struct lightViewProjections
		{
			mat4 lightViewProjection[6];
		};

		ConstBuffer<lightViewProjections> cbProjections;
		cbProjections.create();

		ConstBuffer<vec4> psConstBuffer;
		psConstBuffer.create();

		D3D11_VIEWPORT boundedViewport;
		UINT numViewports = 1u;
		context->RSGetViewports(&numViewports, &boundedViewport);


		context->RSSetViewports(1, &m_viewport);

		static const vec3 directions[6] =
		{
			vec3(1.0f, 0.0f, 0.0f), // +X
			vec3(-1.0f, 0.0f, 0.0f), // -X
			vec3(0.0f, 1.0f, 0.0f), // +Y
			vec3(0.0f, -1.0f, 0.0f), // -Y
			vec3(0.0f, 0.0f, 1.0f), // +Z
			vec3(0.0f, 0.0f, -1.0f) // -Z
		};

		for (size_t i = 0; i < lightPositions.size(); i++)
		{
			context->ClearDepthStencilView(m_plDSVS[i].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0u);

			lightViewProjections constantBufferData{
				viewMatrix(lightPositions[i], directions[0], vec3(0.0f, 1.0f, 0.0f)) * projection,
				viewMatrix(lightPositions[i], directions[1], vec3(0.0f, 1.0f, 0.0f)) * projection,
				viewMatrix(lightPositions[i], directions[2], vec3(0.0f, 0.0f, -1.0f)) * projection,
				viewMatrix(lightPositions[i], directions[3], vec3(0.0f, 0.0f, 1.0f)) * projection,
				viewMatrix(lightPositions[i], directions[4], vec3(0.0f, 1.0f, 0.0f)) * projection,
				viewMatrix(lightPositions[i], directions[5], vec3(0.0f, 1.0f, 1.0f)) * projection,
			};

			vec4 psData(lightPositions[i], m_ProjectionInfo.farPlane);

			cbProjections.updateBuffer(&constantBufferData);
			cbProjections.bind(0u, GS);

			psConstBuffer.updateBuffer(&psData);
			psConstBuffer.bind(10u, PS);

			context->OMSetRenderTargets(0u, nullptr, m_plDSVS[i].Get());

			renderGroup.renderUsingShader(m_plShader);
		}

		context->OMSetRenderTargets(0u, nullptr, nullptr);
		context->RSSetViewports(1u, &boundedViewport);
	}

	template<typename I, typename M>
	inline void ShadowManager::RenderSpotLightShadowMaps(const std::vector<SpotLight>& lights, OpaqueInstances<I, M>& renderGroup)
	{
		if (lights.size() != m_slDSVS.size())
			createPointLightShadowMaps(lights.size());

		auto context = D3D::GetInstance()->GetContext();

		ConstBuffer<mat4> cbProjections;
		cbProjections.create();

		D3D11_VIEWPORT boundedViewport;
		UINT numViewports = 1u;
		context->RSGetViewports(&numViewports, &boundedViewport);

		context->RSSetViewports(1, &m_viewport);

		auto TS = TransformSystem::Init();
		for (size_t i = 0; i < lights.size(); i++)
		{
			vec3 position = lights[i].position;
			if (lights[i].bindedObjectId != -1)
				position += (vec3&)*TS->GetModelTransforms(lights[i].bindedObjectId)[0].modelToWold[3];
			mat4 projection = projectionMatrix(lights[i].cutoffAngle * 2.0f, m_ProjectionInfo.nearPlane, m_ProjectionInfo.farPlane, 1.0f);

			mat4 constantBufferData(viewMatrix(position, lights[i].direction, topFromDir(lights[i].direction)) * projection);

			cbProjections.updateBuffer(&constantBufferData);
			cbProjections.bind(4u, VS);

			context->ClearDepthStencilView(m_slDSVS[i].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0u);
			context->OMSetRenderTargets(0u, nullptr, m_slDSVS[i].Get());

			renderGroup.render();
		}

		context->OMSetRenderTargets(0u, nullptr, nullptr);
		context->RSSetViewports(1u, &boundedViewport);
	}
}

