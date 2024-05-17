#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <string>
#include <unordered_map>
#include <memory>

namespace Engine
{
	struct shader
	{
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
		Microsoft::WRL::ComPtr<ID3D11HullShader> hullShader;
		Microsoft::WRL::ComPtr<ID3D11DomainShader> domainShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader;
		D3D_PRIMITIVE_TOPOLOGY topology;

		bool createPS(ID3DBlob* psBlob);
		bool createVS(ID3DBlob* vsBlob, const D3D11_INPUT_ELEMENT_DESC* ied, UINT iedSize);
		bool createHS(ID3DBlob* psBlob);
		bool createDS(ID3DBlob* psBlob);
		bool createGS(ID3DBlob* psBlob);
		bool CreateShader(ID3DBlob* vsBlob, ID3DBlob* psBlob, const D3D11_INPUT_ELEMENT_DESC* ied, UINT iedSize);
		void BindShader();
		
	};

	class ShaderManager
	{
	public:
		static std::shared_ptr<shader> CompileAndCreateShader(const char* shaderName, const wchar_t* vertexShaderSource, const wchar_t* pixelShaderSource,
			const D3D11_INPUT_ELEMENT_DESC* ied, UINT iedSize, const D3D_SHADER_MACRO* vertexShaderMacro, const D3D_SHADER_MACRO* pixelShaderMacro, 
			D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, const char* vsEntryPoint = "main", const char* psEntryPoint = "main");

		static std::shared_ptr<shader> CompileAndCreateShader(const char* shaderName, const wchar_t* vertexShaderSource, const wchar_t* pixelShaderSource,
			const wchar_t* hullShaderSource, const wchar_t* domainShaderSource, const wchar_t* geometryShaderSource, 
			const D3D11_INPUT_ELEMENT_DESC* ied, UINT iedSize, const D3D_SHADER_MACRO* vertexShaderMacro, const D3D_SHADER_MACRO* pixelShaderMacro, 
			D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, const char* vsEntryPoint = "main", const char* psEntryPoint = "main", 
			const char* hsEntryPoint = "main", const char* dsEntryPoint = "main",const char* gsEntryPoint = "main");

		static std::shared_ptr<shader> GetShader(const char* name);
		static void deleteShader(const char* name);

		static void deleteShaders();

	private:
		static std::unordered_map<std::string, std::shared_ptr<shader>> shaders;
	};
};