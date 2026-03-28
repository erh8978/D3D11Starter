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

// Root constants for bindless resources
struct RayTracingDrawData
{
	unsigned int SceneDataConstantBufferIndex;
	unsigned int EntityDataDescriptorIndex;
	unsigned int SceneTLASDescriptorIndex;
	unsigned int OutputUAVDescriptorIndex;
};

// Overall scene data for ray tracing (constant buffer)
struct RayTracingSceneData
{
	DirectX::XMFLOAT4X4 InverseViewProjection;
	DirectX::XMFLOAT3 CameraPosition;
	float pad;
};

// Per-entity information (geom, material, etc.)
// - Multiple sets of these will be stored in a structured buffer
// - Materials *could* be separated out into their own buffer
//   to cut down on data repetition
struct RayTracingEntityData
{
	DirectX::XMFLOAT4 Color;
	unsigned int VertexBufferDescriptorIndex;
	unsigned int IndexBufferDescriptorIndex;
	float pad[2];
};