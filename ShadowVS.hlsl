#include "ShaderIncludes.hlsli"

// Constant buffer for external (C++) data
cbuffer externalData : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
};

// ------------------------------------------------------
// Simplified vertex shader for rendering to a shadow map
// ------------------------------------------------------
float4 main(VertexShaderInput input) : SV_Position
{
    matrix wvp = mul(projection, mul(view, world));
    return mul(wvp, float4(input.localPosition, 1.0f));
}