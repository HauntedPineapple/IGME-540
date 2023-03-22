#include "ShaderIncludes.hlsli"

TextureCube cubemap: register(t0);
SamplerState samplerOptions : register(s0);

float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}