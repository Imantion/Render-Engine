#pragma once
#include "D3D.h"
#include "Math/math.h"
#include "memory"
#include "Buffers.h"

namespace Engine
{
	class Texture;
	struct shader;
	class Camera;


	class SkyBox
	{
	public:

		SkyBox(Texture* skyBoxTexture, shader* skyBoxShader);
		void BindCamera(const Camera* camera);

		void BindSkyBox(UINT slot);

	private:
		struct cameraFrustrum
		{
			vec4 frustrums[4];
		} cb_data;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_readOnlyDepthBuffer;
		ConstBuffer<cameraFrustrum> m_cb;
		std::shared_ptr<Texture> m_skyBoxTexture;
		std::shared_ptr<shader> m_skyBoxShader;
		const Camera* m_pCamera;
	};
}

