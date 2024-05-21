#include "Graphics/ShaderManager.h"
#include "Graphics/D3D.h"
#include "assert.h"




bool Engine::shader::createPS(ID3DBlob* psBlob)
{
	if (D3D* d3d = D3D::GetInstance())
	{
		HRESULT hr = d3d->GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
		assert(SUCCEEDED(hr));

		if (pixelShader)
			return true;
	}

	return false;
}

bool Engine::shader::createVS(ID3DBlob* vsBlob, const D3D11_INPUT_ELEMENT_DESC* ied, UINT iedSize)
{
	if (D3D* d3d = D3D::GetInstance())
	{

		HRESULT hr = d3d->GetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
		assert(SUCCEEDED(hr));

		hr = d3d->GetDevice()->CreateInputLayout(ied, iedSize, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
		assert(SUCCEEDED(hr));

		if (vertexShader && inputLayout)
			return true;
	}

	return false;
}

bool Engine::shader::createHS(ID3DBlob* psBlob)
{
	if (D3D* d3d = D3D::GetInstance())
	{
		HRESULT hr = d3d->GetDevice()->CreateHullShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &hullShader);
		assert(SUCCEEDED(hr));

		if (hullShader)
			return true;
	}

	return false;
}

bool Engine::shader::createDS(ID3DBlob* psBlob)
{
	if (D3D* d3d = D3D::GetInstance())
	{
		HRESULT hr = d3d->GetDevice()->CreateDomainShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &domainShader);
		assert(SUCCEEDED(hr));

		if (domainShader)
			return true;
	}

	return false;
}

bool Engine::shader::createGS(ID3DBlob* psBlob)
{
	if (D3D* d3d = D3D::GetInstance())
	{
		HRESULT hr = d3d->GetDevice()->CreateGeometryShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &geometryShader);
		assert(SUCCEEDED(hr));

		if (geometryShader)
			return true;
	}

	return false;
}

bool Engine::shader::CreateShader(ID3DBlob* vsBlob, ID3DBlob* psBlob, const D3D11_INPUT_ELEMENT_DESC* ied, UINT iedSize)
{
	bool isSuccessful = createPS(psBlob);
	isSuccessful = isSuccessful && createVS(vsBlob, ied, iedSize);

	return isSuccessful;
}

void Engine::shader::EnableShader()
{
	isEnabled = true;
}

void Engine::shader::DisableShader()
{
	isEnabled = false;
}

void Engine::shader::BindShader()
{
	D3D* d3d = D3D::GetInstance();
	if (d3d && isEnabled)
	{
		d3d->GetContext()->VSSetShader(vertexShader.Get(), nullptr, 0u);
		d3d->GetContext()->PSSetShader(pixelShader.Get(), nullptr, 0u);
		d3d->GetContext()->HSSetShader(hullShader.Get(), nullptr, 0u);
		d3d->GetContext()->DSSetShader(domainShader.Get(), nullptr, 0u);
		d3d->GetContext()->GSSetShader(geometryShader.Get(), nullptr, 0u);
		d3d->GetContext()->IASetInputLayout(inputLayout.Get());
		d3d->GetContext()->IASetPrimitiveTopology(topology);
	}
}




std::unordered_map<std::string, std::shared_ptr<Engine::shader>> Engine::ShaderManager::shaders;

std::shared_ptr<Engine::shader> Engine::ShaderManager::CompileAndCreateShader(const char* shaderName, const wchar_t* vertexShaderSource, const wchar_t* pixelShaderSource,
	const D3D11_INPUT_ELEMENT_DESC* ied, UINT iedSize, const D3D_SHADER_MACRO* vertexShaderMacro,const D3D_SHADER_MACRO* pixelShaderMacro, D3D_PRIMITIVE_TOPOLOGY topology,
	const char* vsEntryPoint, const char* psEntryPoint)
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

	std::shared_ptr<shader> shader = std::make_shared<Engine::shader>();

	if (shader->CreateShader(vertexlBlob.Get(), pixelBlob.Get(), ied, iedSize))
	{
		shader->topology = topology;
		shaders.insert({ shaderName, shader });

		return shader;
	}

	return nullptr;
}

std::shared_ptr<Engine::shader> Engine::ShaderManager::CompileAndCreateShader(const char* shaderName, const wchar_t* vertexShaderSource, 
	const wchar_t* pixelShaderSource, const wchar_t* hullShaderSource, const wchar_t* domainShaderSource, const wchar_t* geometryShaderSource, 
	const D3D11_INPUT_ELEMENT_DESC* ied, UINT iedSize, const D3D_SHADER_MACRO* vertexShaderMacro, const D3D_SHADER_MACRO* pixelShaderMacro, D3D_PRIMITIVE_TOPOLOGY topology,
	const char* vsEntryPoint, const char* psEntryPoint, const char* hsEntryPoint, const char* dsEntryPoint, const char* gsEntryPoint)
{
	UINT flags = 0;
#ifdef _DEBUG 
	flags |= D3DCOMPILE_DEBUG;
#endif

	std::shared_ptr<shader> shader = std::make_shared<Engine::shader>();

	Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;

	HRESULT hr = D3DCompileFromFile(vertexShaderSource, vertexShaderMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, vsEntryPoint, "vs_5_0", flags, 0, &shaderBlob, nullptr);
	assert(SUCCEEDED(hr));
	shader->createVS(shaderBlob.Get(), ied, iedSize);

	hr = D3DCompileFromFile(pixelShaderSource, pixelShaderMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, psEntryPoint, "ps_5_0", flags, 0, &shaderBlob, nullptr);
	assert(SUCCEEDED(hr));
	shader->createPS(shaderBlob.Get());

	if (hullShaderSource != nullptr)
	{
		hr = D3DCompileFromFile(hullShaderSource, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, hsEntryPoint, "hs_5_0", flags, 0, &shaderBlob, nullptr);
		assert(SUCCEEDED(hr));
		shader->createHS(shaderBlob.Get());
	}

	if(domainShaderSource != nullptr)
	{
		hr = D3DCompileFromFile(domainShaderSource, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, dsEntryPoint, "ds_5_0", flags, 0, &shaderBlob, nullptr);
		assert(SUCCEEDED(hr));
		shader->createDS(shaderBlob.Get());
	}

	if(geometryShaderSource != nullptr)
	{
		hr = D3DCompileFromFile(geometryShaderSource, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, gsEntryPoint, "gs_5_0", flags, 0, &shaderBlob, nullptr);
		assert(SUCCEEDED(hr));
		shader->createGS(shaderBlob.Get());
	}

	shader->topology = topology;

	shaders.insert({ shaderName, shader });

	return shader;
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
