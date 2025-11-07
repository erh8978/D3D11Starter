#include "ShaderIncludes.hlsli"
// Debug pixel shader. Returns color based on surface normals.

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 textureScale;
    float2 textureOffset;
    float totalTime;
    float3 cameraPos;
    float roughness;
    float3 backgroundColor;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    input.Normal = normalize(input.Normal);
    
	// Return a color based on the normal
    return float4(input.Normal, 1);
}