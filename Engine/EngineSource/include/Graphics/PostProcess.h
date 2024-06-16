#pragma once
#include "D3D.h"
#include "Buffers.h"
#include <mutex>
#include <memory>

namespace Engine
{
	struct shader;

	class PostProcess
	{
	public:
		static PostProcess* Init();
		static void Deinit();

		void SetLightToColorShader(std::shared_ptr<shader> shader);
		
		void Resolve(ID3D11ShaderResourceView* srv, ID3D11RenderTargetView* dst);

		void AddEV100(float EV100);
		void SetGamma(float gamma);

	private:
		PostProcess();
		~PostProcess() = default;

		static PostProcess* m_instance;
		static std::mutex m_mutex;

		std::shared_ptr<shader> m_lightToColorShader;

		struct postProcessData
		{
			float EV100 = 5.0f;
			float gamma = 2.2f;
			float padding[2];
		} m_postProcessData;

		ConstBuffer<postProcessData> m_constantBuffer;
	};
}