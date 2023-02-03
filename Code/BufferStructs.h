#pragma once

#include <DirectXMath.h>

// This struct has to match the vertex shader's cbuffer's 
// order and type of the variables
struct VertexShaderExternalData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT4X4 worldMatrix;
};