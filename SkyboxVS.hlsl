#include "ShaderIncludes.hlsli"

// Description of constant buffer data
cbuffer ExternalData : register(b0)
{
    matrix projection;
    matrix view;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
SkyboxVertexToPixel main(VertexShaderInput input )
{
	// Set up output struct
	SkyboxVertexToPixel output;
	
	// Copy view matrix, but zero out position
    matrix viewNoPosition = view;
    viewNoPosition._14 = 0;
    viewNoPosition._24 = 0;
    viewNoPosition._34 = 0;
	
    matrix vp = mul(projection, viewNoPosition);
	
    output.screenPosition = mul(vp, float4(input.localPosition, 1)).xyww;
    output.sampleDir = input.localPosition;
	
	return output;
}