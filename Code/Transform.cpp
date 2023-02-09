#include "Transform.h"

using namespace DirectX;

Transform::Transform() :
	m_position(0, 0, 0),
	m_rotation(0, 0, 0),
	m_scale(1, 1, 1),
	m_isMatrixDirty(false)
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

DirectX::XMFLOAT3 Transform::GetRight()
{
	return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	return DirectX::XMFLOAT3();
}

// ================ SETTERS ================
void Transform::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
	m_isMatrixDirty = true;
}
void Transform::SetPosition(DirectX::XMFLOAT3 newPosition) {
	m_position = newPosition;
	m_isMatrixDirty = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	m_rotation.x = pitch;
	m_rotation.y = yaw;
	m_rotation.z = roll;
	m_isMatrixDirty = true;
}
void Transform::SetRotation(DirectX::XMFLOAT3 pitchYawRoll) {
	m_rotation = pitchYawRoll;
	m_isMatrixDirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
	m_scale.x = x;
	m_scale.y = y;
	m_scale.z = z;
	m_isMatrixDirty = true;
}
void Transform::SetScale(DirectX::XMFLOAT3 scale) {
	m_scale = scale;
	m_isMatrixDirty = true;
}


// ================ TRANSFORMERS ================
void Transform::MoveAbsolute(float x, float y, float z)
{
	m_position.x += x;
	m_position.y += y;
	m_position.z += z;
	m_isMatrixDirty = true;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	m_position.x += offset.x;
	m_position.y += offset.y;
	m_position.z += offset.z;
	m_isMatrixDirty = true;
}

void Transform::MoveRelative(float a_x, float a_y, float a_z)
{
	XMVECTOR moveBy = XMVectorSet(a_x, a_y, a_z, 0);
	XMVECTOR currentRotation = XMLoadFloat3(&m_rotation);
	XMVECTOR currentPosition = XMLoadFloat3(&m_position);

	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYawFromVector(currentRotation);
	XMVECTOR relativeOffset = XMVector3Rotate(moveBy, rotQuat);

	currentPosition += relativeOffset;

	XMStoreFloat3(&m_position, currentPosition);
	m_isMatrixDirty = true;
}

void Transform::MoveRelative(DirectX::XMFLOAT3 a_offset)
{
	XMVECTOR moveBy = XMLoadFloat3(&a_offset);
	XMVECTOR currentRotation = XMLoadFloat3(&m_rotation);
	XMVECTOR currentPosition = XMLoadFloat3(&m_position);

	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYawFromVector(currentRotation);
	XMVECTOR relativeOffset = XMVector3Rotate(moveBy, rotQuat);

	currentPosition += relativeOffset;

	XMStoreFloat3(&m_position, currentPosition);
}

void Transform::Rotate(float p, float y, float r)
{
	m_rotation.x += p;
	m_rotation.y += y;
	m_rotation.z += r;
	m_isMatrixDirty = true;
}

void Transform::Rotate(DirectX::XMFLOAT3 pitchYawRoll)
{
	m_rotation.x += pitchYawRoll.x;
	m_rotation.y += pitchYawRoll.y;
	m_rotation.z += pitchYawRoll.z;
	m_isMatrixDirty = true;
}

void Transform::Scale(float x, float y, float z)
{
	m_scale.x *= x;
	m_scale.y *= y;
	m_scale.z *= z;
	m_isMatrixDirty = true;
}

void Transform::Scale(DirectX::XMFLOAT3 scale)
{
	m_scale.x *= scale.x;
	m_scale.y *= scale.y;
	m_scale.z *= scale.z;
	m_isMatrixDirty = true;
}

void Transform::UpdateMatrices()
{
	if (m_isMatrixDirty == false) return;

	XMMATRIX t = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX r = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	XMMATRIX s = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	XMMATRIX world = s * r * t;
	XMStoreFloat4x4(&m_worldMatrix, world);
	XMStoreFloat4x4(&m_worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(world)));

	m_isMatrixDirty = false;
}

void Transform::UpdateVectors()
{
}
