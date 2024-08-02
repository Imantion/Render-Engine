#pragma once
#include "Graphics/D3D.h"
#include "Graphics/MeshSystem.h"
#include "Graphics/LightSystem.h"
#include "Render/Camera.h"
#include <vector>
#include <mutex>

namespace Engine
{
	struct vec3;

	class ShadowSystem
	{
	public:
		ShadowSystem(const ShadowSystem& other) = delete;
		void operator=(const ShadowSystem&) = delete;

		static ShadowSystem* Init();
		static void Deinit();

		void SetShadowShaders(std::shared_ptr<shader> pointLight, std::shared_ptr<shader> spotLight, std::shared_ptr<shader> directionalLight);

		void SetShadowTextureResolution(UINT resolution);
		UINT GetShadowTextureResolution();

		void SetProjectionInfo(float nearPlane, float farPlane);

		template <typename I, typename M>
		void RenderPointLightShadowMaps(const std::vector<vec3>& lightPositions, OpaqueInstances<I, M>& renderGroup);

		template <typename I, typename M>
		void RenderSpotLightShadowMaps(const std::vector<SpotLight>& lights, OpaqueInstances<I, M>& renderGroup);

		template <typename I, typename M>
		void RenderDirectLightShadowMaps(const std::vector<DirectionalLight>& lights, const Camera* camera, OpaqueInstances<I, M>& renderGroup);

		void BindShadowTextures(UINT pointLightTexture, UINT spotLightTexture, UINT directionaLightTexture);
		void BindShadowBuffers(UINT spotLightsBufferSlot, UINT directionalLightsBufferSlot);

	private:
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

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srvDirLigts;
		std::vector <Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> m_dlDSVS;
		std::vector<mat4> m_dlViewProjections;
		ConstBuffer<mat4> m_dlConstBuff;

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
		static ShadowSystem* m_instance;
		static std::mutex m_mutex;
	};







	template<typename I, typename M>
	inline void ShadowSystem::RenderPointLightShadowMaps(const std::vector<vec3>& lightPositions, OpaqueInstances<I, M>& renderGroup)
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
	inline void ShadowSystem::RenderSpotLightShadowMaps(const std::vector<SpotLight>& lights, OpaqueInstances<I, M>& renderGroup)
	{
		if (lights.size() != m_slDSVS.size())
		{
			m_slViewProjections.resize(lights.size());
			Create2DShadowMaps(lights.size(), m_slDSVS, m_srvSpotLigts);
		}
			

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
			{
				auto& transform = TS->GetModelTransforms(lights[i].bindedObjectId)[0].modelToWold;
				position += (vec3&)*transform[3];
				vec3 direction = vec4(lights[i].direction, 0.0f) * transform;

				mat4 projection = projectionMatrix(lights[i].cutoffAngle * 2.0f, m_ProjectionInfo.nearPlane, m_ProjectionInfo.farPlane, 1.0f);
				m_slViewProjections[i] =  viewMatrix(position, direction, topFromDir(direction)) * projection;
			}

			cbProjections.updateBuffer(&m_slViewProjections[i]);
			cbProjections.bind(4u, VS);

			context->ClearDepthStencilView(m_slDSVS[i].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0u);
			context->OMSetRenderTargets(0u, nullptr, m_slDSVS[i].Get());

			renderGroup.renderUsingShader(m_slShader);
		}

		context->OMSetRenderTargets(0u, nullptr, nullptr);
		context->RSSetViewports(1u, &boundedViewport);
	}

	template<typename I, typename M>
	inline void ShadowSystem::RenderDirectLightShadowMaps(const std::vector<DirectionalLight>& lights, const Camera* camera, OpaqueInstances<I, M>& renderGroup)
	{
		if (lights.size() != m_dlDSVS.size())
		{
			m_dlViewProjections.resize(lights.size());
			Create2DShadowMaps(lights.size(), m_dlDSVS, m_srvDirLigts);
		}


		auto context = D3D::GetInstance()->GetContext();

		ConstBuffer<mat4> cbProjections;
		cbProjections.create();

		D3D11_VIEWPORT boundedViewport;
		UINT numViewports = 1u;
		context->RSGetViewports(&numViewports, &boundedViewport);

		context->RSSetViewports(1, &m_viewport);

		vec3 clipSpaceCorners[8] = {
		  vec3(-1.0f, 1.0f, 0.0f),
		  vec3(1.0f,  1.0f, 0.0f), 
		  vec3(-1.0f,-1.0f, 0.0f),
		  vec3(1.0f, -1.0f, 0.0f), 
		  vec3(-1.0f, 1.0f, 1.0f),
		  vec3(1.0f, 1.0f, 1.0f), 
		  vec3(-1.0f, -1.0f, 1.0f), 
		  vec3(1.0f, -1.0f, 1.0f) 
		};

		vec3 frustrumCentr;

		for (size_t i = 0; i < 8; i++)
		{
			vec4 viewSpace = vec4(clipSpaceCorners[i], 1.0f) * camera->getInverseProjectionMatrix();
			clipSpaceCorners[i] = (viewSpace / viewSpace.w) * camera->getInverseViewMatrix();

			frustrumCentr += clipSpaceCorners[i];
		}

		frustrumCentr = frustrumCentr * (1.0f / 8.0f);
		

		float radius = (clipSpaceCorners[2] - clipSpaceCorners[5]).length() * 0.5f;

		mat4 ortProjection = orthographicProjecton(radius, -radius, radius, -radius, -radius * 6.0f, radius * 6.0f);
		for (size_t i = 0; i < lights.size(); i++)
		{
			vec3 eye = frustrumCentr - (lights[i].direction * 2.0f * radius);
			mat4 view = LookAt(eye, frustrumCentr, vec3(0.0f, 1.0f, 0.0f));
			m_dlViewProjections[i] = view * ortProjection;

			cbProjections.updateBuffer(&m_dlViewProjections[i]);

			cbProjections.bind(4u, VS);

			context->ClearDepthStencilView(m_dlDSVS[i].Get(), D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 0.0f, 0u);
			context->OMSetRenderTargets(0, nullptr, m_dlDSVS[i].Get());

			renderGroup.renderUsingShader(m_dlShader);
		}

		m_dlConstBuff.updateBuffer(&m_dlViewProjections[0]);

		context->OMSetRenderTargets(0u, nullptr, nullptr);
		context->RSSetViewports(1u, &boundedViewport);
	}
}

