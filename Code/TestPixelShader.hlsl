#include "ShaderIncludes.hlsli"

#define NUM_LIGHTS 5

cbuffer ExternalData : register(b0)
{
    float3 colorTint;
    float3 ambientColor;
    float3 cameraPosition;
    float roughness;
    float2 uvScale;
    float2 uvOffset;

    Light lights[NUM_LIGHTS]; // Array of exactly NUM_LIGHTS lights
}

Texture2D DiffuseTexture : register(t0); // "t" registers for textures
Texture2D SpecularTexture : register(t1); 
Texture2D ORMTexture : register(t2);
Texture2D NormalMap : register(t3);
SamplerState BasicSampler : register(s0); // "s" registers for samplers

float4 main(VertexToPixel input) : SV_TARGET
{
    input.normal = normalize(input.normal); // Must renormalize any interpolated vectors
    input.uv = input.uv * uvScale + uvOffset;
    //input.tangent = normalize(input.tangent);
    
    float specularScale = SpecularTexture.Sample(BasicSampler, input.uv).r;
    float occlusionLevel = ORMTexture.Sample(BasicSampler, input.uv).r;
    float roughnessLevel = ORMTexture.Sample(BasicSampler, input.uv).g;
    float metallicLevel = ORMTexture.Sample(BasicSampler, input.uv).b;
    
    float3 surfaceColor = DiffuseTexture.Sample(BasicSampler, input.uv).rgb * colorTint;
    float3 finalPixelColor = ambientColor * surfaceColor;
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        switch (lights[i].type)
        {
            case 0: //directional
                finalPixelColor += DirectionalLight(lights[i], surfaceColor, input.normal, cameraPosition, input.worldPosition, roughnessLevel, specularScale);
                break;
            case 1: //point
                finalPixelColor += PointLight(lights[i], surfaceColor, input.normal, cameraPosition, input.worldPosition, roughness, specularScale);
                break;
            case 2: //spot
                break;
        }

    }
    return float4(finalPixelColor, 1);
}