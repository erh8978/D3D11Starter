#include "ShaderIncludes.hlsli"
// Basic pixel shader. Just returns a color tint passed via constant buffer.
cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 textureScale;
    float2 textureOffset;
    float totalTime;
    float3 cameraPos;
    
    Light lights[5]; // Array of exactly 5 lights
}

// Texture and sampler state are bound with registers
Texture2D Albedo		    : register(t0);
Texture2D NormalMap         : register(t1);
Texture2D MetalMap          : register(t2);
Texture2D RoughnessMap      : register(t3);
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
    // (Ortho)normalize vectors as necessary
    input.Normal = normalize(input.Normal);
    input.Tangent = normalize(input.Tangent - dot(input.Tangent, input.Normal) * input.Normal);
    
    // Modify UV coords
    input.UV = input.UV * textureScale + textureOffset;
    
    // Calculate bitangent and create TBN matrix
    float3 Bitangent = cross(input.Tangent, input.Normal);
    float3x3 TBN = float3x3(input.Tangent, Bitangent, input.Normal);
    
    // Sample albedo color and gamma correct it
    float4 albedoColor = GammaCorrect(Albedo.Sample(BasicSampler, input.UV), 2.2);
    
    // Sample normal map, unpack it, and transform it from tangent space to world space with TBN matrix
    float3 finalNormal = mul(normalize(NormalMap.Sample(BasicSampler, input.UV) * 2 - 1).xyz, TBN);
    
    // Sample metal and roughness maps
    float metalness = MetalMap.Sample(BasicSampler, input.UV).r;
    float roughness = RoughnessMap.Sample(BasicSampler, input.UV).r;
    
    
    // Calculate the unit vector to camera
    float3 dirToCamera = normalize(cameraPos - input.worldPosition);
    
    // Calculate specular color
    float3 f0 = lerp(F0_NON_METAL, albedoColor.rgb, metalness);
    
    // Variable to hold sum of lighting calculations
    float3 lightTotal = float3(0.0f, 0.0f, 0.0f);
    
    // Loop for each light
    for (int i = 0; i < 1; i++)
    {
        Light light = lights[i]; // Assign current light to a variable for easy access
        light.Direction = normalize(light.Direction); // Normalize light's direction, if it has one
        
        float3 dirToLight = -light.Direction; // This will be replaced if light is Point or Spot; done to prevent "dirToLight potentially uninitialized" warning
        //float attenuation = 1; // Default to 100% intensity
        
        // Calcualte dirToLight and attenuation for point and spot lights
        if(light.Type == LIGHT_TYPE_POINT || light.Type == LIGHT_TYPE_SPOT)
        {
            dirToLight = normalize(light.Position - input.worldPosition); // Normalized direction to a light in worldspace
            //attenuation = Attenuate(light, input.worldPosition); // Attenuation is only relevant to lights with a range
        }
        // For spot lights ONLY, consider the angle to the light's direction and scale attenuation accordingly
        if(light.Type == LIGHT_TYPE_SPOT)
        {
            float pixelAngle = saturate(dot(-dirToLight, light.Direction));
            float cosOuter = cos(light.SpotOuterAngle);
            float cosInner = cos(light.SpotInnerAngle);
            float falloffRange = cosOuter - cosInner;
            float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
            
            //attenuation *= spotTerm;
        }
        
        // Calculate the light amounts
        float diff = DiffusePBR(finalNormal, dirToLight);
        float3 spec = MicrofacetBRDF(finalNormal, dirToLight, dirToCamera, roughness, f0);
        
        // Calculate diffuse with energy conservation, including cutting diffuse for metals
        float3 h = normalize(dirToCamera + dirToLight);
        float3 F = F_Schlick(dirToCamera, h, f0);
        float3 balancedDiff = DiffuseEnergyConserve(diff, F, metalness);
        
        return spec.rgbb;
        
        // Combine the final diffuse and specular values for this light
        float3 total = (balancedDiff * albedoColor.rgb + spec) * light.Intensity * light.Color;
        
        lightTotal = lightTotal + total;
    }
    
    return GammaCorrect(float4(lightTotal, 1), 1.0 / 2.2);
}