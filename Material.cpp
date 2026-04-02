#include "Material.h"
#include "Graphics.h"
#include "PathHelpers.h"

Material::Material(
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState,
	DirectX::XMFLOAT4 colorTint,
	DirectX::XMFLOAT2 uvScale,
	DirectX::XMFLOAT2 uvOffset)
	:
	_colorTint(colorTint),
	_uvScale(uvScale),
	_uvOffset(uvOffset),
	_pipelineState(pipelineState)
{
}

// Nothing to delete
Material::~Material()
{
}

void Material::LoadTextureSet(std::wstring textureName)
{
	_albedoMapIndex = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/" + textureName + L"_albedo.png").c_str());
	_normalMapIndex = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/" + textureName + L"_normals.png").c_str());
	_metalnessIndex = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/" + textureName + L"_metal.png").c_str());
	_roughnessIndex = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/" + textureName + L"_roughness.png").c_str());
}

DirectX::XMFLOAT4 Material::GetColorTint() { return _colorTint; }
DirectX::XMFLOAT2 Material::GetUVScale() { return _uvScale; }
DirectX::XMFLOAT2 Material::GetUVOffset() { return _uvOffset; }
Microsoft::WRL::ComPtr<ID3D12PipelineState> Material::GetPipelineStateObject() { return _pipelineState; }
unsigned int Material::GetAlbedoMapIndex() { return _albedoMapIndex; }
unsigned int Material::GetNormalMapIndex() { return _normalMapIndex; }
unsigned int Material::GetMetalnessIndex() { return _metalnessIndex; }
unsigned int Material::GetRoughnessIndex() { return _roughnessIndex; }

void Material::SetColorTint(DirectX::XMFLOAT4 colorTint) { _colorTint = colorTint; }
void Material::SetUVScale(DirectX::XMFLOAT2 uvScale) { _uvScale = uvScale; }
void Material::SetUVOffset(DirectX::XMFLOAT2 uvOffset) { _uvOffset = uvOffset; }
void Material::SetPipelineStateObject(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState) { _pipelineState = pipelineState; }
void Material::SetAlbedoMapIndex(unsigned int albedoMapIndex) { _albedoMapIndex = albedoMapIndex; }
void Material::SetNormalMapIndex(unsigned int normalMapIndex) { _normalMapIndex = normalMapIndex; }
void Material::SetMetalnessIndex(unsigned int metalnessIndex) { _metalnessIndex = metalnessIndex; }
void Material::SetRoughnessIndex(unsigned int roughnessIndex) { _roughnessIndex = roughnessIndex; }