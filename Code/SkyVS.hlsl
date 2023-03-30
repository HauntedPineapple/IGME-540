#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;
}

VertexToSkyPixel main(VertexShaderInput input)
{
    // Set up output struct
    VertexToSkyPixel output;

	// Combine all matrices
    matrix viewNoTranslation = viewMatrix;
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;
    
    output.screenPosition = mul(mul(projectionMatrix, viewNoTranslation), float4(input.localPosition, 1.0f));
    output.screenPosition.z = output.screenPosition.w;
    output.sampleDir = input.localPosition;
    
    return output;
}