#include "Graphics/ShaderManager.h"
#include "Graphics/D3D.h"
#include "assert.h"




bool Engine::shader::CreateShader(ID3DBlob* vsBlob, ID3DBlob* psBlob, const D3D11_INPUT_ELEMENT_DESC* ied, UINT iedSize)
{
	{
		if (D3D* d3d = D3D::GetInstance())
		{

			HRESULT hr = d3d->GetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
			assert(SUCCEEDED(hr));

			hr = d3d->GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
			assert(SUCCEEDED(hr));

			hr = d3d->GetDevice()->CreateInputLayout(ied, iedSize, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
			assert(SUCCEEDED(hr));

			if(pixelShader && vertexShader)
			return true;
		}
		
		return false;
	}
}

void Engine::shader::BindShader()
{
	if (D3D* d3d = D3D::GetInstance())
	{
		d3d->GetContext()->VSSetShader(vertexShader.Get(), nullptr, 0u);
		d3d->GetContext()->PSSetShader(pixelShader.Get(), nullptr, 0u);
		d3d->GetContext()->IASetInputLayout(inputLayout.Get());
	}
}




std::unordered_map<std::string, std::shared_ptr<Engine::shader>> Engine::ShaderManager::shaders;

std::shared_ptr<Engine::shader> Engine::ShaderManager::CompileAndCreateShader(const char* shaderName, const wchar_t* vertexShaderSource, const wchar_t* pixelShaderSource,const D3D11_INPUT_ELEMENT_DESC* ied, UINT iedSize,
	const D3D_SHADER_MACRO* vertexShaderMacro,const D3D_SHADER_MACRO* pixelShaderMacro,const char* vsEntryPoint, const char* psEntryPoint)
{
	UINT flags = 0;
#ifdef _DEBUG 
	flags |= D3DCOMPILE_DEBUG;
#endif

	Microsoft::WRL::ComPtr<ID3DBlob> pixelBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> vertexlBlob;
	/*ID3DBlob* vertexlBlob;*/

	HRESULT hr = D3DCompileFromFile(vertexShaderSource, vertexShaderMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, vsEntryPoint, "vs_5_0", flags, 0, &vertexlBlob, nullptr);
	assert(SUCCEEDED(hr));

	hr = D3DCompileFromFile(pixelShaderSource, pixelShaderMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, psEntryPoint, "ps_5_0", flags, 0, &pixelBlob, nullptr);
	assert(SUCCEEDED(hr));

	std::shared_ptr<shader>shader = std::make_shared<Engine::shader>();

	if (shader->CreateShader(vertexlBlob.Get(), pixelBlob.Get(), ied, iedSize))
	{
		shaders.insert({ shaderName, shader });

		return shader;
	}

	return nullptr;
}

std::shared_ptr<Engine::shader> Engine::ShaderManager::GetShader(const char* name)
{
	auto returnValue = shaders.find(name);

	if (returnValue != shaders.end())
		return (*returnValue).second;

	return nullptr;

}

void Engine::ShaderManager::deleteShader(const char* name)
{
	auto it = shaders.find(name);

	if (it != shaders.end())
	{
		shaders.erase(it);
	}
}

void Engine::ShaderManager::deleteShaders()
{
	shaders.clear();
}
