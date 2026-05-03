#pragma once

#include <DirectXMath.h>
#include "Light.h"

struct VertexShaderExternalData
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 worldInvTranspose;
};

struct PixelShaderExternalData
{
	Light lights[10];
	unsigned int albedoMapIndex;
	unsigned int normalMapIndex;
	unsigned int metalnessIndex;
	unsigned int roughnessIndex;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	DirectX::XMFLOAT3 colorTint;
	float padding;
	DirectX::XMFLOAT3 cameraPos;
};

struct SkyboxVertexShaderExternalData
{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

struct SkyboxPixelShaderExternalData
{
	unsigned int skyTextureIndex;
	DirectX::XMFLOAT3 sunDir;
};

struct SunRaysPixelShaderExternalData
{
	unsigned int colorIndex;
	unsigned int sunVisibilityIndex;
	unsigned int normalsIndex;
	unsigned int depthIndex;
	DirectX::XMFLOAT4 screenSunPos;
};