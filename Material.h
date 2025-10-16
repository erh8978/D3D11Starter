#pragma once
#include<DirectXMath.h>
#include<wrl/client.h>
#include<d3d11.h>

class Material
{
public:

	Material(DirectX::XMFLOAT4 colorTint,
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader,
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader);
	~Material();

	DirectX::XMFLOAT4 GetColorTint();
	void SetColorTint(DirectX::XMFLOAT4 newColorTint);

	Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();
	void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> newVertexShader);

	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();
	void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> newPixelShader);

private:
	
	DirectX::XMFLOAT4 myColorTint;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> myVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> myPixelShader;
};

