#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <string>
#include <unordered_map>

namespace Engine
{
	struct shader
	{
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

		bool CreateShader(ID3DBlob* vsBlob, ID3DBlob* psBlob, const D3D11_INPUT_ELEMENT_DESC* ied);
		void BindShader();
		
	};

	class ShaderManager
	{
	public:
		static shader* CompileAndCreateShader(const char* shaderName, const wchar_t* vertexShaderSource, const wchar_t* pixelShaderSource, const D3D11_INPUT_ELEMENT_DESC* ied, const D3D_SHADER_MACRO* vertexShaderMacro,
										   const D3D_SHADER_MACRO* pixelShaderMacro);

		static shader* GetShader(const char* name);
		static void deleteShader(const char* name);

		static void deleteShaders();

	private:
		static std::unordered_map<std::string, shader*> shaders;
	};
};