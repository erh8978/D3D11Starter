#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <string>

class Material
{
public:
	Material(
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState,
		DirectX::XMFLOAT4 colorTint = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT2 uvScale = DirectX::XMFLOAT2(1.0f, 1.0f),
		DirectX::XMFLOAT2 uvOffset = DirectX::XMFLOAT2(0.0f, 0.0f));
	~Material();

	void LoadTextureSet(std::wstring textureName);

	// Getters
	DirectX::XMFLOAT4 GetColorTint();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineStateObject();
	unsigned int GetAlbedoMapIndex();
	unsigned int GetNormalMapIndex();
	unsigned int GetMetalnessIndex();
	unsigned int GetRoughnessIndex();

	// Setters
	void SetColorTint(DirectX::XMFLOAT4 colorTint);
	void SetUVScale(DirectX::XMFLOAT2 uvScale);
	void SetUVOffset(DirectX::XMFLOAT2 uvOffset);
	void SetPipelineStateObject(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState);
	void SetAlbedoMapIndex(unsigned int albedoMapIndex);
	void SetNormalMapIndex(unsigned int normalMapIndex);
	void SetMetalnessIndex(unsigned int metalnessIndex);
	void SetRoughnessIndex(unsigned int roughnessIndex);

private:
	// Tint, scale, and offset data
	DirectX::XMFLOAT4 _colorTint;
	DirectX::XMFLOAT2 _uvScale;
	DirectX::XMFLOAT2 _uvOffset;

	// Pipeline state (instead of pixel and vertex shaders)
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;

	// Texture indices (for bindless texture indexing)
	// Look at how nice and aligned they are :3
	unsigned int _albedoMapIndex = -1;
	unsigned int _normalMapIndex = -1;
	unsigned int _metalnessIndex = -1;
	unsigned int _roughnessIndex = -1;
};

