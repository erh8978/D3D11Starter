#include "Material.h"

Material::Material(DirectX::XMFLOAT4 colorTint,
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader,
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader)
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
