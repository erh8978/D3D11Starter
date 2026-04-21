#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    unsigned int skyTextureIndex;
}

SamplerState BasicSampler : register(s0);

float4 main(SkyboxVertexToPixel input) : SV_TARGET
{
    TextureCube SkyTexture = ResourceDescriptorHeap[skyTextureIndex];
    return SkyTexture.Sample(BasicSampler, input.sampleDir);
};