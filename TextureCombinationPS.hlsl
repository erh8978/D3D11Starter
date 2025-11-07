#include "ShaderIncludes.hlsli"
// Texture combination pixel shader. Takes two textures

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

// Texture and sampler state are bound with registers
Texture2D BottomTexture     : register(t0);
Texture2D TopTexture        : register(t1);
SamplerState BasicSampler   : register(s0);

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
	// Modify UV coordinates with scale and offset
    float2 modifiedUV = input.UV * textureScale + textureOffset;
	
	// Get base color by sampling the texture
    float4 bottomColor = BottomTexture.Sample(BasicSampler, modifiedUV);
    float4 topColor = TopTexture.Sample(BasicSampler, modifiedUV);
    // Combine by multiplying
    float4 surfaceColor = bottomColor * topColor;
	
	// Multiply by color tint to get the result
    return surfaceColor * colorTint;
}