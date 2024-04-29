#include "Graphics/ShaderManager.h"
#include "Graphics/D3D.h"

#include "assert.h"
bool Engine::shader::CreateShader(ID3DBlob* vsBlob, ID3DBlob* psBlob)
{
	{
		if (D3D* d3d = D3D::GetInstance())
		{
			vertexShaderBlob.Attach(vsBlob);

			HRESULT hr = d3d->GetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
			assert(SUCCEEDED(hr));

			hr = d3d->GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
			assert(SUCCEEDED(hr));
			
			if(pixelShader && vertexShader)
			return true;
		}
		
		return false;
	}
}


std::unordered_map<std::string, Engine::shader*> Engine::ShaderManager::shaders;

Engine::shader* Engine::ShaderManager::CompileAndCreateShader(const char* shaderName, const wchar_t* vertexShaderSource, const wchar_t* pixelShaderSource, const D3D_SHADER_MACRO* vertexShaderMacro,const D3D_SHADER_MACRO* pixelShaderMacro)
{
	Microsoft::WRL::ComPtr<ID3DBlob> pixelBlob;
	/*Microsoft::WRL::ComPtr<ID3DBlob> vertexlBlob;*/
	ID3DBlob* vertexlBlob;

	HRESULT hr = D3DCompileFromFile(vertexShaderSource, vertexShaderMacro, nullptr, "main", "vs_5_0", 0, 0, &vertexlBlob, nullptr);
	assert(SUCCEEDED(hr));

	hr = D3DCompileFromFile(pixelShaderSource, pixelShaderMacro, nullptr, "main", "ps_5_0", 0, 0, &pixelBlob, nullptr);
	assert(SUCCEEDED(hr));

	shader* shader = new Engine::shader();

	if (shader->CreateShader(vertexlBlob, pixelBlob.Get()))
	{
		shaders.insert({ shaderName, shader });

		return shader;
	}

	delete shader;
	return nullptr;
}

Engine::shader* Engine::ShaderManager::GetShader(const char* name)
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
		delete (*it).second;
		shaders.erase(it);
	}
}
