#include "ShaderIncludes.hlsli"

cbuffer ExternalData:register(b0)
{
	float4 colorTint;
	float time;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(colorTint.r, Random(float2(time, input.uv.y)), colorTint.b, 0);
}