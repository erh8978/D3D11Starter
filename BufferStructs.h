#pragma once

#include <DirectXMath.h>

struct VertexShaderExternalData
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 worldInvTranspose;
};

struct PixelShaderExternalData
{
	unsigned int albedoMapIndex;
	unsigned int normalMapIndex;
	unsigned int metalnessIndex;
	unsigned int roughnessIndex;
};