#include "ShaderIncludes.hlsli"

#define NUM_LIGHTS 5

cbuffer ExternalData : register(b0)
{
    float gamma;
    float roughness;
    float3 colorTint;
    float3 ambientColor;
    float3 cameraPosition;
    float2 uvScale;
    float2 uvOffset;
    int useSpecularMap;

    Light lights[NUM_LIGHTS]; // Array of exactly NUM_LIGHTS lights
}

Texture2D DiffuseTexture : register(t0); // "t" registers for textures
Texture2D NormalMap : register(t1);
Texture2D MetalMap : register(t2);
Texture2D RoughnessMap : register(t3);
Texture2D ShadowMap : register(t4);
SamplerState BasicSampler : register(s0); // "s" registers for samplers
SamplerComparisonState ShadowSampler : register(s1);

float4 main(VertexToPixel input) : SV_TARGET
{
    // Perform the perspective divide (divide by W) ourselves
    input.shadowMapPos /= input.shadowMapPos.w;
    // Convert the normalized device coordinates to UVs for sampling
    float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
    shadowUV.y = 1 - shadowUV.y; // Flip the Y
    // Grab the distances we need: light-to-pixel and closest-surface
    float distToLight = input.shadowMapPos.z;
    // float distShadowMap = ShadowMap.Sample(BasicSampler, shadowUV).r;
    // For testing, just return black where there are shadows.
    //if (distShadowMap < distToLight) 
    //    return float4(0, 0, 0, 1);
    // Get a ratio of comparison results using SampleCmpLevelZero()
    float shadowAmount = ShadowMap.SampleCmpLevelZero(
    ShadowSampler,
    shadowUV,
    distToLight).r;
    
    // Must renormalize any interpolated vectors
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    input.uv = input.uv * uvScale + uvOffset;

    // Normal mapping
    float3 unpackedNormal = normalize(NormalMap.Sample(BasicSampler, input.uv).rgb * 2.0f - 1.0f);
    // rotate the normal map to convert from tangent to world space
    float3 N = input.normal;
    float3 T = input.tangent;
    T = normalize(T - N * dot(input.tangent, N)); // ensure we ortho-normalize the tangent again
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    // multiply normal map vector by TBN
    input.normal = mul(unpackedNormal, TBN);

    // Un-Gamma correct diffuse texture
    float3 surfaceColor = pow(DiffuseTexture.Sample(BasicSampler, input.uv).rgb, gamma) * colorTint;

    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    float metalness = MetalMap.Sample(BasicSampler, input.uv).r;
    float3 specularColor = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);
    
    float3 finalPixelColor = ambientColor * surfaceColor;
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        switch (lights[i].type)
        {
            case 0: //directional
                finalPixelColor += DirectionalLightPBR(lights[i], surfaceColor, input.normal, cameraPosition, input.worldPosition, roughness, metalness, specularColor);
                if (i == 0)
                {
                    finalPixelColor *= shadowAmount;
                }
                break;
            case 1: //point
                finalPixelColor += PointLightPBR(lights[i], surfaceColor, input.normal, cameraPosition, input.worldPosition, roughness, metalness, specularColor);
                break;
            case 2: //spot
                break;
        }
    }
    
    // Apply gamma correction
    finalPixelColor = pow(finalPixelColor, 1.0f / gamma);
    return float4(finalPixelColor, 1);
}