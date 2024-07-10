#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "Utils\EnumDefinitions.h"

namespace Engine
{
	struct shader
	{
		bool create(ID3DBlob* blob, shaderTypes shaderType);

		void EnableShader();
		void DisableShader();
		void BindInputLyout(ID3D11InputLayout* layout);
		void BindShader();

		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		Microsoft::WRL::ComPtr<ID3DBlob> vertexBlob;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
		Microsoft::WRL::ComPtr<ID3D11HullShader> hullShader;
		Microsoft::WRL::ComPtr<ID3D11DomainShader> domainShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader;
		D3D_PRIMITIVE_TOPOLOGY topology;
		
		bool isEnabled = true;

	private:
		ID3D11InputLayout* inputLayout = nullptr;
		
	};

	class ShaderManager
	{
	public:
		static std::shared_ptr<shader> CompileAndCreateShader(const char* shaderName, const wchar_t* vertexShaderSource, const wchar_t* pixelShaderSource,
			const D3D_SHADER_MACRO* vertexShaderMacro, const D3D_SHADER_MACRO* pixelShaderMacro, 
			D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, const char* vsEntryPoint = "main", const char* psEntryPoint = "main");

		static std::shared_ptr<shader> CompileAndCreateShader(const char* shaderName, const wchar_t* vertexShaderSource, const wchar_t* pixelShaderSource,
			const wchar_t* hullShaderSource, const wchar_t* domainShaderSource, const wchar_t* geometryShaderSource, 
			const D3D_SHADER_MACRO* vertexShaderMacro, const D3D_SHADER_MACRO* pixelShaderMacro, 
			D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, const char* vsEntryPoint = "main", const char* psEntryPoint = "main", 
			const char* hsEntryPoint = "main", const char* dsEntryPoint = "main",const char* gsEntryPoint = "main");

		static ID3D11InputLayout* CreateInputLayout(const char* InputLayouName, ID3DBlob* vsBlob, const D3D11_INPUT_ELEMENT_DESC* ied, UINT iedSize);

		static std::shared_ptr<shader> GetShader(const char* name);
		static void deleteShader(const char* name);

		static void deleteShaders();

	private:
		static std::unordered_map<std::string, std::shared_ptr<shader>> shaders;
		static std::unordered_map <std::string, Microsoft::WRL::ComPtr<ID3D11InputLayout>> inputLayouts;
	};
};