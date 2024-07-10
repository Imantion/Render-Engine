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

		SkyBox();
		SkyBox(std::shared_ptr<Texture> skyBoxTexture, std::shared_ptr<shader> skyBoxShader);

		void SetTexture(std::shared_ptr<Texture>);
		void SetShader(std::shared_ptr<shader>);

		void BindCamera(const Camera* camera);
		void BindSkyBox(UINT slot);

		void Draw();

	private:

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_readOnlyDepthBuffer;
		std::shared_ptr<Texture> m_skyBoxTexture;
		std::shared_ptr<shader> m_skyBoxShader;
		const Camera* m_pCamera;
	};
}

