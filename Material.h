#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <unordered_map>

class Material
{
public:

	Material(DirectX::XMFLOAT4 colorTint,
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader,
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader,
		float roughnessValue = 0.5f);
	~Material();

	DirectX::XMFLOAT4 GetColorTint();
	void SetColorTint(DirectX::XMFLOAT4 newColorTint);

	Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();
	void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> newVertexShader);

	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();
	void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> newPixelShader);

	void AddTextureSRV(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV);
	void AddSamplerState(unsigned int slot, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

	void BindTexturesAndSamplers();

	void SetTextureScale(DirectX::XMFLOAT2 scale);
	DirectX::XMFLOAT2 GetTextureScale();

	void SetTextureOffset(DirectX::XMFLOAT2 offset);
	DirectX::XMFLOAT2 GetTextureOffset();

	float roughness;

private:
	
	DirectX::XMFLOAT4 myColorTint;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> myVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> myPixelShader;
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
	DirectX::XMFLOAT2 textureScale;
	DirectX::XMFLOAT2 textureOffset;
};

