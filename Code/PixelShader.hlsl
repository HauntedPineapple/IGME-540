#include "ShaderIncludes.hlsli"

#define NUM_LIGHTS 5

cbuffer ExternalData : register(b0)
{
    float gamma;
    float roughness;
    float3 colorTint;
    float3 ambientColor;
    float3 cameraPosition;

    Light lights[NUM_LIGHTS]; // Array of exactly NUM_LIGHTS lights
}

Texture2D ShadowMap : register(t0);
SamplerState BasicSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

float4 main(VertexToPixel input) : SV_TARGET
{
	// Perform the perspective divide (divide by W) ourselves
    input.shadowMapPos /= input.shadowMapPos.w;
    // Convert the normalized device coordinates to UVs for sampling
    float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
    shadowUV.y = 1 - shadowUV.y; // Flip the Y
    float distToLight = input.shadowMapPos.z;
    float shadowAmount = ShadowMap.SampleCmpLevelZero(
    ShadowSampler,
    shadowUV,
    distToLight).r;
	
    input.normal = normalize(input.normal); // Must renormalize any interpolated vectors
    float3 finalPixelColor = ambientColor * colorTint;

    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        switch (lights[i].type)
        {
            case 0: //directional
                finalPixelColor += DirectionalLight(lights[i], colorTint, input.normal, cameraPosition, input.worldPosition, roughness, 1);
                if (i == 0)
                {
                    finalPixelColor *= shadowAmount;
                }
                break;
            case 1: //point
                finalPixelColor += PointLight(lights[i], colorTint, input.normal, cameraPosition, input.worldPosition, roughness, 1);
                break;
            case 2: //spot
                break;
        }

    }
    finalPixelColor = pow(finalPixelColor, 1.0f / gamma);
    return float4(finalPixelColor, 1);
}