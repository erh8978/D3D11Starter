#pragma once
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#include <DirectXMath.h>

struct Light
{
	int Type;						// Type of light - see above
	DirectX::XMFLOAT3 Direction;	// For Directional and Spot
	float Range;					// Max range of Point and Spot attenuation
	DirectX::XMFLOAT3 Position;		// World space position for Point and Spot
	float Intensity;				// All types need an intensity
	DirectX::XMFLOAT3 Color;		// All types need a color
	float SpotInnerAngle;			// Inner cone angle (in radians) for Spot - within this angle, full intensity
	float SpotOuterAngle;			// Outer cone angle (in radians) for Spot - outside this angle, no light
	DirectX::XMFLOAT2 Padding;		// Intentional padding to hit the 16-byte boundary for HLSL

	inline static Light Directional(DirectX::XMFLOAT3 direction, float intensity, DirectX::XMFLOAT3 color)
	{
		Light result = {};
		result.Type = LIGHT_TYPE_DIRECTIONAL;
		result.Direction = direction;
		result.Intensity = intensity;
		result.Color = color;

		return result;
	}

	inline static Light Point(DirectX::XMFLOAT3 position, float intensity, DirectX::XMFLOAT3 color, float range = 5.0f)
	{
		Light result = {};
		result.Type = LIGHT_TYPE_POINT;
		result.Range = range;
		result.Position = position;
		result.Intensity = intensity;
		result.Color = color;

		return result;
	}

	inline static Light Spot(DirectX::XMFLOAT3 direction, DirectX::XMFLOAT3 position, float intensity, DirectX::XMFLOAT3 color, float range = 5.0f, float innerConeRadians = 0.1f, float outerConeRadians = 0.5f)
	{
		Light result = {};
		result.Type = LIGHT_TYPE_SPOT;
		result.Direction = direction;
		result.Range = range;
		result.Position = position;
		result.Intensity = intensity;
		result.Color = color;
		result.SpotInnerAngle = innerConeRadians;
		result.SpotOuterAngle = outerConeRadians;

		return result;
	}
};