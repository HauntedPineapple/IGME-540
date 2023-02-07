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
void Transform::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
}
void Transform::SetPosition(DirectX::XMFLOAT3 newPosition) { m_position = newPosition; }

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	m_rotation.x = pitch;
	m_rotation.y = yaw;
	m_rotation.z = roll;
}
void Transform::SetRotation(DirectX::XMFLOAT3 pitchYawRoll) { m_rotation = pitchYawRoll; }

void Transform::SetScale(float x, float y, float z)
{
	m_scale.x = x;
	m_scale.y = y;
	m_scale.z = z;
}
void Transform::SetScale(DirectX::XMFLOAT3 scale) { m_scale = scale; }


// ================ TRANSFORMERS ================
void Transform::MoveAbsolute(float x, float y, float z)
{
	m_position.x += x;
	m_position.y += y;
	m_position.z += z;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	m_position.x += offset.x;
	m_position.y += offset.y;
	m_position.z += offset.z;
}

void Transform::Rotate(float p, float y, float r)
{
	m_rotation.x += p;
	m_rotation.y += y;
	m_rotation.z += r;
}

void Transform::Rotate(DirectX::XMFLOAT3 pitchYawRoll)
{
	m_rotation.x += pitchYawRoll.x;
	m_rotation.y += pitchYawRoll.y;
	m_rotation.z += pitchYawRoll.z;
}

void Transform::Scale(float x, float y, float z)
{
	m_scale.x *= x;
	m_scale.y *= y;
	m_scale.z *= z;
}

void Transform::Scale(DirectX::XMFLOAT3 scale)
{
	m_scale.x *= scale.x;
	m_scale.y *= scale.y;
	m_scale.z *= scale.z;
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
