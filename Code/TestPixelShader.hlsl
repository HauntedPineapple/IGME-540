#include "ShaderIncludes.hlsli"

#define NUM_LIGHTS 5

cbuffer ExternalData : register(b0)
{
    float roughness;
    float3 colorTint;
    float3 ambientColor;
    float3 cameraPosition;
    float2 uvScale;
    float2 uvOffset;

    Light lights[NUM_LIGHTS]; // Array of exactly NUM_LIGHTS lights
}

Texture2D DiffuseTexture : register(t0); // "t" registers for textures
Texture2D SpecularTexture : register(t1); // "t" registers for textures
SamplerState BasicSampler : register(s0); // "s" registers for samplers

float4 main(VertexToPixel input) : SV_TARGET
{
    input.normal = normalize(input.normal); // Must renormalize any interpolated vectors
    input.uv = input.uv * uvScale + uvOffset;
    
    float specularScale = SpecularTexture.Sample(BasicSampler, input.uv).r;
    
    float3 surfaceColor = DiffuseTexture.Sample(BasicSampler, input.uv).rgb * colorTint;
    float3 finalPixelColor = ambientColor * surfaceColor;
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        switch (lights[i].type)
        {
            case 0: //directional
                finalPixelColor += DirectionalLight(lights[i], surfaceColor, input.normal, cameraPosition, input.worldPosition, roughness, specularScale);
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