#pragma once
#include <DirectXMath.h>

struct VertexShaderData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
};
