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
	DirectX::XMFLOAT3 GetRight();
	DirectX::XMFLOAT3 GetUp();
	DirectX::XMFLOAT3 GetForward();

	// Setters
	void SetPosition(float a_x, float a_y, float a_z);
	void SetPosition(DirectX::XMFLOAT3 a_position);
	void SetRotation(float a_pitch, float a_yaw, float a_roll);
	void SetRotation(DirectX::XMFLOAT3 pitchYawRoll);
	void SetScale(float a_x, float a_y, float a_z);
	void SetScale(DirectX::XMFLOAT3 a_scale);

	// Transformers
	void MoveAbsolute(float a_x, float a_y, float a_z);
	void MoveAbsolute(DirectX::XMFLOAT3 a_offset);
	void MoveRelative(float a_x, float a_y, float a_z);
	void MoveRelative(DirectX::XMFLOAT3 a_offset);
	void Rotate(float a_pitch, float a_yaw, float a_roll);
	void Rotate(DirectX::XMFLOAT3 a_pitchYawRoll);
	void Scale(float a_x, float a_y, float a_z);
	void Scale(DirectX::XMFLOAT3 a_scale);

private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMFLOAT3 m_scale;

	DirectX::XMFLOAT3 m_right;
	DirectX::XMFLOAT3 m_up;
	DirectX::XMFLOAT3 m_forward;

	DirectX::XMFLOAT4X4 m_worldMatrix;
	DirectX::XMFLOAT4X4 m_worldInverseTransposeMatrix;

	bool m_isMatricesChanged;
	bool m_isVectorsChanged;

	// Helpers
	void UpdateMatrices();
	void UpdateVectors();
};