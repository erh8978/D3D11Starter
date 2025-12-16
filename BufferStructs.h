#pragma once
#include <DirectXMath.h>
#include "Lights.h"

struct VertexShaderExternalData
{
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 worldInvTranspose;
	DirectX::XMFLOAT4X4 lightView;
	DirectX::XMFLOAT4X4 lightProjection;
};

struct PixelShaderExternalData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT2 textureScale;
	DirectX::XMFLOAT2 textureOffset;
	float totalTime;
	DirectX::XMFLOAT3 cameraPos;
	Light lights[5];
};

struct SkyboxVertexShaderExternalData
{
	DirectX::XMFLOAT4X4 projectionMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
};

struct BlurPixelShaderExternalData
{
	int blurRadius;
	float pixelWidth;
	float pixelHeight;
};

struct PixelizationPixelShaderExternalData
{
	int pixelSize;
	DirectX::XMFLOAT2 windowSize;
};