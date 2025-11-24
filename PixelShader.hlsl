#include "ShaderIncludes.hlsli"
// Basic pixel shader. Just returns a color tint passed via constant buffer.

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 textureScale;
    float2 textureOffset;
    float totalTime;
    float3 cameraPos;
    float roughness;
    float3 ambientColor;
    
    Light lights[5]; // Array of exactly 5 lights
}

// Texture and sampler state are bound with registers, specifically t0, t1, and s0
Texture2D SurfaceTexture		: register(t0);
Texture2D NormalMap             : register(t1);
SamplerState BasicSampler		: register(s0);

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
    // Normalize vectors as necessary
    input.Normal = normalize(input.Normal);
    
    // Modify UV coordinates with scale and offset
    input.UV = input.UV * textureScale + textureOffset;
    
    // Orthonormalize tangent
    input.Tangent = normalize(input.Tangent - dot(input.Tangent, input.Normal) * input.Normal);
    
    // Calculate bitangent
    float3 Bitangent = cross(input.Tangent, input.Normal);
    
    // Create TBN matrix
    float3x3 TBN = float3x3(input.Tangent, Bitangent, input.Normal);
    
    // Sample normal map
    float3 packedNormal = (float3)NormalMap.Sample(BasicSampler, input.UV);
    float3 unpackedNormal = normalize(packedNormal * 2 - 1);
    
    // Transform normal from map
    float3 finalNormal = mul(unpackedNormal, TBN);
    
    // Calculate the unit vector to the camera
    float3 dirToCamera = normalize(cameraPos - input.worldPosition);
    
    // Get base color by sampling the texture
    float4 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.UV);
    // Then gamma correct it
    surfaceColor = GammaCorrect(surfaceColor, 2.2);
    
    // Ambient term = ambientColor * surface color
    float3 ambientTerm = ambientColor * surfaceColor.rgb;
    
    // Calcualte our "shininess" value using the roughness of the material
    float specExponent = max((1.0f - roughness) * MAX_SPECULAR_EXPONENT, 1);
    
    // Keep a running total of all light on this pixel, starting with the ambient term because it's only added once
    float3 lightTotal = ambientTerm;
    // Do the following for EACH light:
    for (int i = 0; i < 5; i++)
    {
        Light light = lights[i];
        
        light.Direction = normalize(light.Direction);
        
        float3 dirToLight = -light.Direction;
        float attenuation = 1;
        switch (light.Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                //dirToLight = -light.Direction;
            
                break;
            case LIGHT_TYPE_POINT:
                dirToLight = normalize(light.Position - input.worldPosition);
                attenuation = Attenuate(light, input.worldPosition);
            
                break;
            case LIGHT_TYPE_SPOT:
                dirToLight = normalize(light.Position - input.worldPosition);
            
                float pixelAngle = saturate(dot(-dirToLight, light.Direction));
                float cosOuter = cos(light.SpotOuterAngle);
                float cosInner = cos(light.SpotInnerAngle);
                float falloffRange = cosOuter - cosInner;
            
                float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
            
                attenuation = Attenuate(light, input.worldPosition) * spotTerm;
                break;
        }
        
        // Diffuse term = dot product of the direction to the light along the surface normal * light color * light intensity * surface color
        float3 diffuseTerm = saturate(dot(finalNormal, dirToLight)) * light.Color * light.Intensity * surfaceColor.rgb;
        
        // Reflect INCOMING light direction
        float3 refl = reflect(light.Direction, finalNormal);
        
        // Calculate cosine of the reflection and camera vector
        float RdotV = saturate(dot(refl, dirToCamera));
        
        // Raise to a power equal to some "shininess"
        float3 specularTerm = pow(max(RdotV, 0.0f), specExponent) * light.Color * light.Intensity;
        
        // Cut the specular if the diffuse contribution is zero
        // - any() returns 1 if any component of the param is non-zero
        // - In other words:
        // - If the diffuse amount is 0, any(diffuse) returns 0
        // - If the diffuse amount is != 0, any(diffuse) returns 1
        // - So when diffuse is 0, specular becomes 0
        specularTerm *= any(diffuseTerm);
        
        lightTotal = lightTotal + (diffuseTerm + specularTerm) * attenuation;
    }
    
    return GammaCorrect(float4(lightTotal, 1), 1.0 / 2.2);
}