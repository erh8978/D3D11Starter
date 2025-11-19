#include "ShaderIncludes.hlsli"

TextureCube SkyTexture      : register(t0);
SamplerState BasicSampelr   : register(s0);

float4 main(SkyboxVertexToPixel input) : SV_TARGET
{
    return SkyTexture.Sample(BasicSampelr, input.sampleDir);
};