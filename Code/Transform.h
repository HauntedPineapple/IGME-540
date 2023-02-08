#pragma once

#include <DirectXMath.h>

class Transform
{
public:
	Transform();

	// Getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

	// Setters
	void SetPosition(float a_fX, float a_fY, float a_fZ);
	void SetPosition(DirectX::XMFLOAT3 a_v3Position);
	void SetRotation(float a_fPitch, float a_fYaw, float a_fRoll);
	void SetRotation(DirectX::XMFLOAT3 v3PitchYawRoll);
	void SetScale(float a_fX, float a_fY, float a_fZ);
	void SetScale(DirectX::XMFLOAT3 a_v3Scale);

	// Transformers
	void MoveAbsolute(float a_fX, float a_fY, float a_fZ);
	void MoveAbsolute(DirectX::XMFLOAT3 a_v3Offset);
	//void MoveRelative(float a_fX, float a_fY, float a_fZ);
	//void MoveRelative(DirectX::XMFLOAT3 a_v3Offset);
	void Rotate(float a_fPitch, float a_fYaw, float a_fRoll);
	void Rotate(DirectX::XMFLOAT3 v3PitchYawRoll);
	void Scale(float a_fX, float a_fY, float a_fZ);
	void Scale(DirectX::XMFLOAT3 a_v3Scale);

private:
	DirectX::XMFLOAT3 m_v3Position;
	DirectX::XMFLOAT3 m_v3Rotation;
	DirectX::XMFLOAT3 m_v3Scale;
	DirectX::XMFLOAT4X4 m_m4WorldMatrix;
	DirectX::XMFLOAT4X4 m_m4WorldInverseTransposeMatrix;

	// Helpers
	void UpdateMatrices();
};