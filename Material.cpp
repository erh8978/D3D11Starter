#include "Material.h"
#include "Graphics.h"

Material::Material(DirectX::XMFLOAT4 colorTint,
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader,
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader) :
	textureScale { 1.0f, 1.0f },
	textureOffset { 0.0f, 0.0f }
{
	myColorTint = colorTint;
	myVertexShader = vertexShader;
	myPixelShader = pixelShader;
}

Material::~Material()
{
	// Don't need to do anything here, for now
}

DirectX::XMFLOAT4 Material::GetColorTint()
{
	return myColorTint;
}

void Material::SetColorTint(DirectX::XMFLOAT4 newColorTint)
{
	myColorTint = newColorTint;
}

Microsoft::WRL::ComPtr<ID3D11VertexShader> Material::GetVertexShader()
{
	return myVertexShader;
}

void Material::SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> newVertexShader)
{
	myVertexShader = newVertexShader;
}

Microsoft::WRL::ComPtr<ID3D11PixelShader> Material::GetPixelShader()
{
	return myPixelShader;
}

void Material::SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> newPixelShader)
{
	myPixelShader = newPixelShader;
}

void Material::AddTextureSRV(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV)
{
	textureSRVs[slot] = textureSRV;
}

void Material::AddSamplerState(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers[slot] = sampler;
}

void Material::BindTexturesAndSamplers()
{
	for (const auto& [slot, srv] : textureSRVs)
	{
		Graphics::Context->PSSetShaderResources(slot, 1, srv.GetAddressOf());
	}

	for (const auto& [slot, sampler] : samplers)
	{
		Graphics::Context->PSSetSamplers(slot, 1, sampler.GetAddressOf());
	}
}

void Material::SetTextureScale(DirectX::XMFLOAT2 scale)
{
	textureScale = scale;
}

DirectX::XMFLOAT2 Material::GetTextureScale()
{
	return textureScale;
}

void Material::SetTextureOffset(DirectX::XMFLOAT2 offset)
{
	textureOffset = offset;
}

DirectX::XMFLOAT2 Material::GetTextureOffset()
{
	return textureOffset;
}
