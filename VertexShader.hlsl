#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix worldInvTranspose;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixel output;
	
	// Create WPV matrix by combining world, projection, and view matrices
    matrix wpv = mul(projection, mul(view, world));

	// Here we're essentially passing the input position directly through to the next
	// stage (rasterizer), though it needs to be a 4-component vector now.  
	// - To be considered within the bounds of the screen, the X and Y components 
	//   must be between -1 and 1.  
	// - The Z component must be between 0 and 1.  
	// - Each of these components is then automatically divided by the W component, 
	//   which we're leaving at 1.0 for now (this is more useful when dealing with 
	//   a perspective projection matrix, which we'll get to in the future).
    output.screenPosition = mul(wpv, float4(input.localPosition, 1.0f));
	
	// Pass through other data
    output.UV = input.UV;
    output.Normal = mul((float3x3) worldInvTranspose, input.Normal);
    output.Tangent = mul((float3x3) world, input.Tangent);
    output.worldPos = mul(world, float4(input.localPosition, 1)).xyz;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}