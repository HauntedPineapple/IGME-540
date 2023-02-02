#pragma once

#include <DirectXMath.h>

class Transform
{
public:
	Transform();

	// Getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetPitchYawRoll();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

	// Setters
	void SetPosition(float x, float y, float z);
	void SetPosition(DirectX::XMFLOAT3 position);
	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(DirectX::XMFLOAT3 pitchYawRoll);
	void SetScale(float x, float y, float z);
	void SetScale(DirectX::XMFLOAT3 scale);

	// Transformers
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(DirectX::XMFLOAT3 offset);
	//void MoveRelative(float x, float y, float z);
	//void MoveRelative(DirectX::XMFLOAT3 offset);
	void Rotate(float p, float y, float r);
	void Rotate(DirectX::XMFLOAT3 pitchYawRoll);
	void Scale(float x, float y, float z);
	void Scale(DirectX::XMFLOAT3 scale);

private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_pitchYawRoll;
	DirectX::XMFLOAT3 m_scale;
	DirectX::XMFLOAT4X4 m_worldMatrix;
	DirectX::XMFLOAT4X4 m_worldInverseTransposeMatrix;

	// Helpers
	void UpdateMatrices();
};