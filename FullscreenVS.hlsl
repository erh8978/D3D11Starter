// Copied from D3D11 version; hopefully it works
struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 UV       : TEXCOORD0;
};

VertexToPixel main(uint id : SV_VertexID)
{
    VertexToPixel output;
    
    // Calculate the UV (0, 0) to (2, 2) using the ID
    output.UV = float2(
        (id << 1) & 2, // Essentially: id % 2 * 2
        id & 2);

    // Calculate the position based on the UV
    output.position = float4(output.UV, 0, 1);
    output.position.x = output.position.x * 2 - 1;
    output.position.y = output.position.y * -2 + 1;
    
    return output;
}