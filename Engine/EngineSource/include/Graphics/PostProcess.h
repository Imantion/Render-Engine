#pragma once
#include "D3D.h"
#include "Buffers.h"
#include <mutex>
#include <memory>

namespace Engine
{
	struct shader;
	struct vec4;

	class PostProcess
	{
	public:
		struct FXAAData
		{
			float imageSize[4]; // .xy = image_size, .zw = 1.0 / image_size

			float qualitySubpix = 0.75f;				//   FXAA_QUALITY__SUBPIX, range [0.0; 1.0], default 0.75
			//   Choose the amount of sub-pixel aliasing removal. This can effect sharpness.
			//   1.00 - upper limit (softer)
			//   0.75 - default amount of filtering
			//   0.50 - lower limit (sharper, less sub-pixel aliasing removal)
			//   0.25 - almost off
			//   0.00 - completely off

			float qualityEdgeThreshold = 0.166f;  		//   FXAA_QUALITY__EDGE_THRESHOLD, range [0.063; 0.333], best quality 0.063
			//   The minimum amount of local contrast required to apply algorithm.
			//   0.333 - too little (faster)
			//   0.250 - low quality
			//   0.166 - default
			//   0.125 - high quality 
			//   0.063 - overkill (slower)

			float qualityEdgeThresholdMin = 0.0625f;

			float padding;
		};
	public:
		static PostProcess* Init();
		static void Deinit();

		void SetLightToColorShader(std::shared_ptr<shader> shader);
		void SetFXAAShader(std::shared_ptr<shader> shader);
		
		void Resolve(ID3D11ShaderResourceView* srv, ID3D11RenderTargetView* dst);
		void FXAA(ID3D11ShaderResourceView* srv, ID3D11RenderTargetView* dst);

		void AddEV100(float EV100);
		void SetGamma(float gamma);
		void UpdateFXAABuffer(float qualitySubpix, float qualityEdgeThreshold, float qualityEdgeThresholdMin);

		void SetRTSize(UINT width, UINT Height);

	private:
		PostProcess();
		~PostProcess() = default;

		static PostProcess* m_instance;
		static std::mutex m_mutex;

		std::shared_ptr<shader> m_lightToColorShader;
		std::shared_ptr<shader> m_FXAAShader;

		struct postProcessData
		{
			float EV100 = 2.0f;
			float gamma = 2.2f;
			float padding[2];
		} m_postProcessData;

		 FXAAData m_FXAAData;

		ConstBuffer<postProcessData> m_constantBuffer;
		ConstBuffer<FXAAData> m_FXAAconstantBuffer;

		UINT m_renderTargetWidth;
		UINT m_renderTargetHeight;
	};
}