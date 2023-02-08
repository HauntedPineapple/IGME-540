#include "Transform.h"

using namespace DirectX;

Transform::Transform() :
	m_v3Position(0, 0, 0),
	m_v3Rotation(0, 0, 0),
	m_v3Scale(1, 1, 1)
{
	XMStoreFloat4x4(&m_m4WorldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_m4WorldInverseTransposeMatrix, XMMatrixIdentity());
}

// ================ GETTERS ================
DirectX::XMFLOAT3 Transform::GetPosition() { return m_v3Position; }

DirectX::XMFLOAT3 Transform::GetRotation() { return m_v3Rotation; }

DirectX::XMFLOAT3 Transform::GetScale() { return m_v3Scale; }

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	UpdateMatrices();
	return m_m4WorldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	UpdateMatrices();
	return m_m4WorldInverseTransposeMatrix;
}

// ================ SETTERS ================
void Transform::SetPosition(float a_fX, float a_fY, float a_fZ)
{
	m_v3Position.x = a_fX;
	m_v3Position.y = a_fY;
	m_v3Position.z = a_fZ;
}
void Transform::SetPosition(DirectX::XMFLOAT3 a_v3Position) { m_v3Position = a_v3Position; }

void Transform::SetRotation(float a_fPitch, float a_fYaw, float a_fRoll)
{
	m_v3Rotation.x = a_fPitch;
	m_v3Rotation.y = a_fYaw;
	m_v3Rotation.z = a_fRoll;
}
void Transform::SetRotation(DirectX::XMFLOAT3 a_fPitchYawRoll) { m_v3Rotation = a_fPitchYawRoll; }

void Transform::SetScale(float a_fX, float a_fY, float a_fZ)
{
	m_v3Scale.x = a_fX;
	m_v3Scale.y = a_fY;
	m_v3Scale.z = a_fZ;
}
void Transform::SetScale(DirectX::XMFLOAT3 a_v3Scale) { m_v3Scale = a_v3Scale; }


// ================ TRANSFORMERS ================
void Transform::MoveAbsolute(float a_fX, float a_fY, float a_fZ)
{
	m_v3Position.x += a_fX;
	m_v3Position.y += a_fY;
	m_v3Position.z += a_fZ;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 a_fOffset)
{
	m_v3Position.x += a_fOffset.x;
	m_v3Position.y += a_fOffset.y;
	m_v3Position.z += a_fOffset.z;
}

void Transform::Rotate(float a_fPitch, float a_fYaw, float a_fRoll)
{
	m_v3Rotation.x += a_fPitch;
	m_v3Rotation.y += a_fYaw;
	m_v3Rotation.z += a_fRoll;
}

void Transform::Rotate(DirectX::XMFLOAT3 a_fPitchYawRoll)
{
	m_v3Rotation.x += a_fPitchYawRoll.x;
	m_v3Rotation.y += a_fPitchYawRoll.y;
	m_v3Rotation.z += a_fPitchYawRoll.z;
}

void Transform::Scale(float a_fX, float a_fY, float a_fZ)
{
	m_v3Scale.x *= a_fX;
	m_v3Scale.y *= a_fY;
	m_v3Scale.z *= a_fZ;
}

void Transform::Scale(DirectX::XMFLOAT3 a_v3Scale)
{
	m_v3Scale.x *= a_v3Scale.x;
	m_v3Scale.y *= a_v3Scale.y;
	m_v3Scale.z *= a_v3Scale.z;
}

void Transform::UpdateMatrices()
{
	XMMATRIX t = XMMatrixTranslation(m_v3Position.x, m_v3Position.y, m_v3Position.z);
	XMMATRIX r = XMMatrixRotationRollPitchYaw(m_v3Rotation.x, m_v3Rotation.y, m_v3Rotation.z);
	XMMATRIX s = XMMatrixScaling(m_v3Scale.x, m_v3Scale.y, m_v3Scale.z);

	XMMATRIX world = s * r * t;
	XMStoreFloat4x4(&m_m4WorldMatrix, world);
	XMStoreFloat4x4(&m_m4WorldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(world)));
}
