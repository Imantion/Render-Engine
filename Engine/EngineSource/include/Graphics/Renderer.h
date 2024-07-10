#pragma once
#include "Graphics/D3D.h"
#include "Graphics/Buffers.h"
#include "Math/matrix.h"

namespace Engine
{
	class Camera;

	struct PerFrameCB
	{
		float g_resolution[4];
		float g_time;

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

	protected:
		Renderer();
	private:
		ConstBuffer<PerFrameCB> perFrameBuffer;
		PerFrameCB perFrameData;

		ConstBuffer<PerViewCB> perViewBuffer;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTarget;
		Microsoft::WRL::ComPtr <ID3D11DepthStencilView> pViewDepth;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
	private:
		static std::mutex mutex_;
		static Renderer* pInstance;

	};
}
