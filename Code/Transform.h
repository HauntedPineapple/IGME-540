#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class Transform
{
public:
	Transform();

	// Getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorldMatrix();

	// Setters
	void SetPosition(float x, float y, float z);
	void SetPosition(DirectX::XMFLOAT3 newPosition);

	// Transformers
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(DirectX::XMFLOAT3 offset);
private:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 pitchYawRoll;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;
};