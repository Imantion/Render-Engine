#pragma once
#include "Graphics/D3D.h"
#include "Graphics/Buffers.h"
#include "Math/matrix.h"

namespace Engine
{
	class Camera;
	class Texture;

	struct PerFrameCB
	{
		float g_resolution[4];
		float g_time;
		int diffuse = 1;
		int specular  = 1;
		int IBL = 1;
		int LTC = 1;
		float padding[3];
	};

	struct PerViewCB
	{
		mat4 ProjectedView;
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
		void ReleaseRenderTarget() { pRenderTarget.ReleaseAndGetAddressOf(); }
		void updatePerFrameCB(float deltaTime,float wWidth,float wHeight);

		void Render(Camera* camera);
		void PostProcess();

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
	private:
		ConstBuffer<PerFrameCB> perFrameBuffer;
		PerFrameCB perFrameData;

		ConstBuffer<PerViewCB> perViewBuffer;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pHDRtextureResource;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pHDRRenderTarget;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTarget;
		Microsoft::WRL::ComPtr <ID3D11DepthStencilView> pViewDepth;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;

		std::shared_ptr<Texture> diffuseIBL;
		std::shared_ptr<Texture> specularIBL;
		std::shared_ptr<Texture> reflectanceIBL;

		std::shared_ptr<Texture> LTCmat;
		std::shared_ptr<Texture> LTCamp;
	private:
		static std::mutex mutex_;
		static Renderer* pInstance;

	};
}
