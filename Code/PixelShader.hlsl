#include "ShaderIncludes.hlsli"

#define NUM_LIGHTS 5

cbuffer ExternalData : register(b0)
{
	float roughness;
	float3 colorTint;
	float3 ambientColor;
	float3 cameraPosition;

	Light lights[NUM_LIGHTS]; // Array of exactly NUM_LIGHTS lights
}

float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal); // Must renormalize any interpolated vectors
	float3 finalPixelColor = ambientColor * colorTint;

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		switch (lights[i].type)
		{
			case 0: //directional
				finalPixelColor += DirectionalLight(lights[i], colorTint, input.normal, cameraPosition, input.worldPosition, roughness,1);
				break;
			case 1: //point
				finalPixelColor += PointLight(lights[i], colorTint, input.normal, cameraPosition, input.worldPosition, roughness,1);
				break;
			case 2: //spot
				break;
		}

	}
	return float4(finalPixelColor, 1);
}