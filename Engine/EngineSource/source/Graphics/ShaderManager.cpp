#include "Graphics/ShaderManager.h"
#include "Graphics/D3D.h"
#include "assert.h"




bool Engine::shader::create(ID3DBlob* blob, shaderTypes shaderType)
{
	auto device = D3D::GetInstance()->GetDevice();
	HRESULT hr = 0;
	switch (shaderType)
	{
	case Engine::shaderTypes::VS:
		hr = device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vertexShader);
		break;
	case Engine::shaderTypes::HS:
		hr = device->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &hullShader);
		break;
	case Engine::shaderTypes::DS:
		hr = device->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &domainShader);
		break;
	case Engine::shaderTypes::GS:
		hr = device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &geometryShader);
		break;
	case Engine::shaderTypes::PS:
		hr = device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixelShader);
		break;
	default:
		break;
	}

	if(SUCCEEDED(hr))
		return true;

	assert(SUCCEEDED(hr));
	return false;

	
}

void Engine::shader::EnableShader()
{
	isEnabled = true;
}

void Engine::shader::DisableShader()
{
	isEnabled = false;
}

void Engine::shader::BindInputLyout(ID3D11InputLayout* layout)
{
	inputLayout = layout;
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
		if(inputLayout != nullptr)
			d3d->GetContext()->IASetInputLayout(inputLayout);
		d3d->GetContext()->IASetPrimitiveTopology(topology);
	}
}




std::unordered_map<std::string, std::shared_ptr<Engine::shader>> Engine::ShaderManager::shaders;
std::unordered_map <std::string, Microsoft::WRL::ComPtr<ID3D11InputLayout>> Engine::ShaderManager::inputLayouts;

std::shared_ptr<Engine::shader> Engine::ShaderManager::CompileAndCreateShader(const char* shaderName, const wchar_t* vertexShaderSource, const wchar_t* pixelShaderSource,
	const D3D_SHADER_MACRO* vertexShaderMacro,const D3D_SHADER_MACRO* pixelShaderMacro, D3D_PRIMITIVE_TOPOLOGY topology,
	const char* vsEntryPoint, const char* psEntryPoint)
{
	auto existingShader = shaders.find(shaderName);
	if (existingShader != shaders.end())
	{
		std::string tempString = shaderName;
		std::wstring message = L"Shader with name " + std::wstring(tempString.begin(), tempString.end()) + L" alredy exists";
		MessageBox(nullptr, message.c_str(), L"Warning", MB_OK);

		return existingShader->second;
	}

	UINT flags = 0;
#ifdef _DEBUG 
	flags |= D3DCOMPILE_DEBUG;
#endif



	Microsoft::WRL::ComPtr<ID3DBlob> pixelBlob;
	std::shared_ptr<shader> shader = std::make_shared<Engine::shader>();

	HRESULT hr = D3DCompileFromFile(vertexShaderSource, vertexShaderMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, vsEntryPoint, "vs_5_0", flags, 0, &shader->vertexBlob, nullptr);
	assert(SUCCEEDED(hr));

	hr = D3DCompileFromFile(pixelShaderSource, pixelShaderMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, psEntryPoint, "ps_5_0", flags, 0, &pixelBlob, nullptr);
	assert(SUCCEEDED(hr));

	if (shader->create(shader->vertexBlob.Get(), shaderTypes::VS) && shader->create(pixelBlob.Get(),shaderTypes::PS))
	{
		shader->topology = topology;
		shaders.insert({ shaderName, shader });

		return shader;
	}

	return nullptr;
}

std::shared_ptr<Engine::shader> Engine::ShaderManager::CompileAndCreateShader(const char* shaderName, const wchar_t* vertexShaderSource, 
	const wchar_t* pixelShaderSource, const wchar_t* hullShaderSource, const wchar_t* domainShaderSource, const wchar_t* geometryShaderSource, 
	const D3D_SHADER_MACRO* vertexShaderMacro, const D3D_SHADER_MACRO* pixelShaderMacro, D3D_PRIMITIVE_TOPOLOGY topology,
	const char* vsEntryPoint, const char* psEntryPoint, const char* hsEntryPoint, const char* dsEntryPoint, const char* gsEntryPoint)
{
	auto existingShader = shaders.find(shaderName);
	if (existingShader != shaders.end())
	{
		std::string tempString = shaderName;
		std::wstring message = L"Shader with name " + std::wstring(tempString.begin(), tempString.end()) + L" alredy exists";
		MessageBox(nullptr, message.c_str(), L"Warning", MB_OK);

		return existingShader->second;
	}

	UINT flags = 0;
#ifdef _DEBUG 
	flags |= D3DCOMPILE_DEBUG;
#endif

	std::shared_ptr<shader> shader = std::make_shared<Engine::shader>();

	Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;

	HRESULT hr = D3DCompileFromFile(vertexShaderSource, vertexShaderMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, vsEntryPoint, "vs_5_0", flags, 0, &shader->vertexBlob, nullptr);
	assert(SUCCEEDED(hr));
	shader->create(shader->vertexBlob.Get(), shaderTypes::VS);

	hr = D3DCompileFromFile(pixelShaderSource, pixelShaderMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, psEntryPoint, "ps_5_0", flags, 0, &shaderBlob, nullptr);
	assert(SUCCEEDED(hr));
	shader->create(shaderBlob.Get(), shaderTypes::PS);

	if (hullShaderSource != nullptr)
	{
		hr = D3DCompileFromFile(hullShaderSource, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, hsEntryPoint, "hs_5_0", flags, 0, &shaderBlob, nullptr);
		assert(SUCCEEDED(hr));
		shader->create(shaderBlob.Get(), shaderTypes::HS);
	}

	if(domainShaderSource != nullptr)
	{
		hr = D3DCompileFromFile(domainShaderSource, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, dsEntryPoint, "ds_5_0", flags, 0, &shaderBlob, nullptr);
		assert(SUCCEEDED(hr));
		shader->create(shaderBlob.Get(), shaderTypes::DS);
	}

	if(geometryShaderSource != nullptr)
	{
		hr = D3DCompileFromFile(geometryShaderSource, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, gsEntryPoint, "gs_5_0", flags, 0, &shaderBlob, nullptr);
		assert(SUCCEEDED(hr));
		shader->create(shaderBlob.Get(), shaderTypes::GS);
	}

	shader->topology = topology;

	shaders.insert({ shaderName, shader });

	return shader;
}

ID3D11InputLayout* Engine::ShaderManager::CreateInputLayout(const char* InputLayoutName, ID3DBlob* vsBlob, const D3D11_INPUT_ELEMENT_DESC* ied, UINT iedSize)
{
	auto existingLayout = inputLayouts.find(InputLayoutName);
	if (existingLayout != inputLayouts.end())
	{
		std::string tempString = InputLayoutName;
		std::wstring message = L"Input layout with name " + std::wstring(tempString.begin(), tempString.end()) + L" alredy exists";
		MessageBox(nullptr, message.c_str(), L"Warning", MB_OK);

		return existingLayout->second.Get();
	}

	inputLayouts[InputLayoutName] = Microsoft::WRL::ComPtr<ID3D11InputLayout>();
	D3D::GetInstance()->GetDevice()->CreateInputLayout(ied, iedSize, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayouts[InputLayoutName]);

	return inputLayouts[InputLayoutName].Get();
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
	inputLayouts.clear();
}
