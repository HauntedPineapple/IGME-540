#include "Transform.h"

using namespace DirectX;

Transform::Transform() :
	m_position(0, 0, 0),
	m_rotation(0, 0, 0),
	m_scale(1, 1, 1)
{
	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_worldInverseTransposeMatrix, XMMatrixIdentity());
}

// ================ GETTERS ================
DirectX::XMFLOAT3 Transform::GetPosition() { return m_position; }

DirectX::XMFLOAT3 Transform::GetRotation() { return m_rotation; }

DirectX::XMFLOAT3 Transform::GetScale() { return m_scale; }

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	UpdateMatrices();
	return m_worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	UpdateMatrices();
	return m_worldInverseTransposeMatrix;
}

// ================ SETTERS ================
void Transform::SetPosition(float a_fX, float a_fY, float a_fZ)
{
	m_position.x = a_fX;
	m_position.y = a_fY;
	m_position.z = a_fZ;
}
void Transform::SetPosition(DirectX::XMFLOAT3 a_v3Position) { m_position = a_v3Position; }

void Transform::SetRotation(float a_fPitch, float a_fYaw, float a_fRoll)
{
	m_rotation.x = a_fPitch;
	m_rotation.y = a_fYaw;
	m_rotation.z = a_fRoll;
}
void Transform::SetRotation(DirectX::XMFLOAT3 a_fPitchYawRoll) { m_rotation = a_fPitchYawRoll; }

void Transform::SetScale(float a_fX, float a_fY, float a_fZ)
{
	m_scale.x = a_fX;
	m_scale.y = a_fY;
	m_scale.z = a_fZ;
}
void Transform::SetScale(DirectX::XMFLOAT3 a_v3Scale) { m_scale = a_v3Scale; }


// ================ TRANSFORMERS ================
void Transform::MoveAbsolute(float a_fX, float a_fY, float a_fZ)
{
	m_position.x += a_fX;
	m_position.y += a_fY;
	m_position.z += a_fZ;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 a_fOffset)
{
	m_position.x += a_fOffset.x;
	m_position.y += a_fOffset.y;
	m_position.z += a_fOffset.z;
}

void Transform::Rotate(float a_fPitch, float a_fYaw, float a_fRoll)
{
	m_rotation.x += a_fPitch;
	m_rotation.y += a_fYaw;
	m_rotation.z += a_fRoll;
}

void Transform::Rotate(DirectX::XMFLOAT3 a_fPitchYawRoll)
{
	m_rotation.x += a_fPitchYawRoll.x;
	m_rotation.y += a_fPitchYawRoll.y;
	m_rotation.z += a_fPitchYawRoll.z;
}

void Transform::Scale(float a_fX, float a_fY, float a_fZ)
{
	m_scale.x *= a_fX;
	m_scale.y *= a_fY;
	m_scale.z *= a_fZ;
}

void Transform::Scale(DirectX::XMFLOAT3 a_v3Scale)
{
	m_scale.x *= a_v3Scale.x;
	m_scale.y *= a_v3Scale.y;
	m_scale.z *= a_v3Scale.z;
}

void Transform::UpdateMatrices()
{
	XMMATRIX t = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX r = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	XMMATRIX s = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	XMMATRIX world = s * r * t;
	XMStoreFloat4x4(&m_worldMatrix, world);
	XMStoreFloat4x4(&m_worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(world)));
}
