#include "ShaderIncludes.hlsli"

// Constant buffer
cbuffer ExternalData : register(b0)
{
    matrix worldMatrix;
    matrix worldInvTransposeMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
    VertexToPixel output;

	// Combine all matrices
    matrix wvp = mul(mul(projectionMatrix, viewMatrix), worldMatrix);

	//output.screenPosition = mul(worldMatrix, float4(input.localPosition, 1.0f));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

    output.uv = input.uv;
	
	// Convert normals from local to world
    output.normal = mul((float3x3) worldInvTransposeMatrix, input.normal);
	
	// get the pixel's world position
    output.worldPosition = mul(worldMatrix, float4(input.localPosition, 1.0f)).xyz;
	
    output.tangent = normalize(mul((float3x3) worldMatrix, input.tangent));

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
    return output;
}