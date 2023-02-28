#include "ShaderIncludes.hlsli"

#define NUM_LIGHTS 3

cbuffer ExternalData : register(b0)
{
    float roughness;
    float3 colorTint;
    float3 ambientColor;
    float3 cameraPosition;

    //Light directionalLightA;
    //Light directionalLightB;
    //Light directionalLightC;
    
    Light lights[NUM_LIGHTS]; // Array of exactly NUM_LIGHTS lights
}

float4 main(VertexToPixel input) : SV_TARGET
{
    input.normal = normalize(input.normal); // Must renormalize any interpolated vectors
    float3 finalPixelColor = ambientColor * colorTint;
    
    //    float3 viewVector = normalize(cameraPosition - input.worldPosition); // vector from surface to camera
    //    // Light A
    //    finalPixelColor += DiffuseBRDF(input.normal, normalize(-directionalLightA.direction)) * directionalLightA.color;
    //    finalPixelColor += SpecularBRDF(input.normal, normalize(directionalLightA.direction), viewVector, roughness) * directionalLightA.color;
    //    finalPixelColor *= colorTint;
    //    // Light B
    //    finalPixelColor += DiffuseBRDF(input.normal, normalize(-directionalLightB.direction)) * directionalLightB.color;
    //    finalPixelColor += SpecularBRDF(input.normal, normalize(directionalLightB.direction), viewVector, roughness) * directionalLightB.color;
    //    finalPixelColor *= colorTint;
    //    // Light C
    //    finalPixelColor += DiffuseBRDF(input.normal, normalize(-directionalLightC.direction)) * directionalLightC.color;
    //    finalPixelColor += SpecularBRDF(input.normal, normalize(directionalLightC.direction), viewVector, roughness) * directionalLightC.color;
    //    finalPixelColor *= colorTint;

    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        switch (lights[i].type)
        {
            case 0: //directional
                finalPixelColor += DirectionalLight(lights[i], colorTint, input.normal, cameraPosition, input.worldPosition, roughness);
                break;
                
            case 1: //point
                finalPixelColor += PointLight(lights[i], colorTint, input.normal, cameraPosition, input.worldPosition, roughness);
                break;
                
            case 2: //spot
                break;
        }
        
    }
    return float4(finalPixelColor, 1);
}