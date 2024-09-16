#pragma once
#include "Graphics/D3D.h"
#include "Graphics/Buffers.h"
#include "Math/matrix.h"

namespace Engine
{
	class Camera;
	class Texture;
	class SkyBox;
	struct shader;

	struct PerFrameCB
	{
		float g_resolution[4];
		float g_time;
		int diffuse = 1;
		int specular  = 1;
		int IBL = 1;
		int LTC = 1;
		float shadowResolution;
		float pointLightFarPlan;
		uint32_t samplesAmount;
		float farClip;
		float nearClip;
		float texelWidth;
		float texelHeight;
	};

	struct PerViewCB
	{
		mat4 ProjectedView;
		mat4 inverseViewProjection;
		mat4 viewMatrix;
		vec4 BL, Top, Right;
		vec3 camerPosition;
		float padding;
		
	};
	class Renderer
	{

	public:
		static Renderer* Init();
		static void Deinit();
		static Renderer* GetInstance() { return pInstance; }

		void InitDepthWithRTV(ID3D11Resource* RenderBuffer, UINT wWidth, UINT wHeight);
		void InitDepth(UINT wWidth, UINT wHeight);
		void InitGBuffer(UINT wWidth, UINT wHeight);
		void FillGBuffer();
		void ReleaseRenderTarget() { pFXAARenderTarget.ReleaseAndGetAddressOf(); }
		void updatePerFrameCB(float deltaTime,float wWidth,float wHeight, float farCLip, float nearClip);
		void CreateNoMSDepth();

		void RenderParticles(Camera* camera);
		void Render(Camera* camera);
		void RenderDecals();
		void PostProcess();
		void FXAA();

		void setSkyBox(std::shared_ptr<SkyBox> skybox);
		void setIBLLight(std::shared_ptr<Texture> diffuse, std::shared_ptr<Texture> specular, std::shared_ptr<Texture> reflectance);
		void setLTCLight(std::shared_ptr<Texture> invMatrix, std::shared_ptr<Texture> amplitude);

		void setIBLLghtState(bool state);
		void setDiffuseState(bool state);
		void setSpecularState(bool state);
		void setLTCState(bool state);

		bool& getIBLLghtState() { return (bool&)perFrameData.IBL; }
		bool& getDiffuseState() { return (bool&)perFrameData.diffuse; }
		bool& getSpecularState() { return (bool&)perFrameData.specular; }
		bool& getLTCState() { return (bool&)perFrameData.LTC; }

	protected:
		Renderer();

		void Shadows(const Camera* camera);
	private:
		struct GBuffer
		{
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Albedo;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> RoughMetal;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Normals;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SecondNormals;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Emmision;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ObjectId;

			Microsoft::WRL::ComPtr<ID3D11Texture2D> normalsTexture;
			Microsoft::WRL::ComPtr<ID3D11Texture2D> secondNormalsTexture;

			void Bind(UINT startSlot, ID3D11ShaderResourceView* normals = nullptr) // if normals == nullptr. Normals will be set
			{
				lastSlot = startSlot;

				if (!normals)
					normals = Normals.Get();
				auto context = D3D::GetInstance()->GetContext();
				ID3D11ShaderResourceView* resources[5] = { Albedo.Get(), RoughMetal.Get(), normals, Emmision.Get(), ObjectId.Get() };
				context->PSSetShaderResources(startSlot, 5u, resources);
			}

			void Unbind()
			{
				if (lastSlot != -1)
				{
					auto context = D3D::GetInstance()->GetContext();
					ID3D11ShaderResourceView* resources[5] = {NULL,NULL,NULL,NULL,NULL};
					context->PSSetShaderResources(lastSlot, 5u, resources);
				}
			}

			int lastSlot = -1;
		} m_GBuffer;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GBufferRTVs[5];
	private:
		ConstBuffer<PerFrameCB> perFrameBuffer;
		PerFrameCB perFrameData;

		ConstBuffer<PerViewCB> perViewBuffer;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pHDRtextureResource;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pHDRRenderTarget;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTarget;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pRenderTargetSRV;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pFXAARenderTarget;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pViewDepth;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pDepthSRV;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencilTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSStencilOnlyState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSReadOnlyState;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pNoMSDepthSRV;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pNoMSDepthStencil;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerState;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pCullFrontRasterizerState;
		Microsoft::WRL::ComPtr<ID3D11BlendState> pBlendState;

		std::shared_ptr<Texture> diffuseIBL;
		std::shared_ptr<Texture> specularIBL;
		std::shared_ptr<Texture> reflectanceIBL;

		std::shared_ptr<Texture> LTCmat;
		std::shared_ptr<Texture> LTCamp;

		std::shared_ptr<SkyBox> pSkyBox;

		uint32_t samplesAmount = 1;

		std::shared_ptr<shader> depthShader;
	private:
		static std::mutex mutex_;
		static Renderer* pInstance;

	};
}
