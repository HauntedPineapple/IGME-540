#include "Transform.h"

using namespace DirectX;

Transform::Transform() :
	m_position(0, 0, 0),
	m_rotation(0, 0, 0),
	m_scale(1, 1, 1),
	m_isMatricesChanged(false),
	m_isVectorsChanged(false)
{
	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_worldInverseTransposeMatrix, XMMatrixIdentity());

	XMStoreFloat3(&m_right, XMVectorSet(1, 0, 0, 0));
	XMStoreFloat3(&m_up, XMVectorSet(0, 1, 0, 0));
	XMStoreFloat3(&m_forward, XMVectorSet(0, 0, 1, 0));
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
	UpdateVectors();
	return m_right;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	UpdateVectors();
	return m_up;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	UpdateVectors();
	return m_forward;
}


// ================ SETTERS ================
void Transform::SetPosition(float a_x, float a_y, float a_z)
{
	m_position.x = a_x;
	m_position.y = a_y;
	m_position.z = a_z;
	m_isMatricesChanged = true;
}

void Transform::SetPosition(DirectX::XMFLOAT3 newPosition) {
	m_position = newPosition;
	m_isMatricesChanged = true;
}

void Transform::SetRotation(float a_pitch, float a_yaw, float a_roll)
{
	m_rotation.x = a_pitch;
	m_rotation.y = a_yaw;
	m_rotation.z = a_roll;
	m_isMatricesChanged = m_isVectorsChanged = true;
}
void Transform::SetRotation(DirectX::XMFLOAT3 a_pitchYawRoll) {
	m_rotation = a_pitchYawRoll;
	m_isMatricesChanged = m_isVectorsChanged = true;
}

void Transform::SetScale(float a_x, float a_y, float a_z)
{
	m_scale.x = a_x;
	m_scale.y = a_y;
	m_scale.z = a_z;
	m_isMatricesChanged = true;
}

void Transform::SetScale(DirectX::XMFLOAT3 a_scale) {
	m_scale = a_scale;
	m_isMatricesChanged = true;
}


// ================ TRANSFORMERS ================
void Transform::MoveAbsolute(float a_x, float a_y, float a_z)
{
	m_position.x += a_x;
	m_position.y += a_y;
	m_position.z += a_z;
	m_isMatricesChanged = true;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 a_offset)
{
	m_position.x += a_offset.x;
	m_position.y += a_offset.y;
	m_position.z += a_offset.z;
	m_isMatricesChanged = true;
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
	m_isMatricesChanged = true;
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
	m_isMatricesChanged = true;
}

void Transform::Rotate(float a_p, float a_y, float a_r)
{
	m_rotation.x += a_p;
	m_rotation.y += a_y;
	m_rotation.z += a_r;
	m_isMatricesChanged = m_isVectorsChanged = true;
}

void Transform::Rotate(DirectX::XMFLOAT3 a_pitchYawRoll)
{
	m_rotation.x += a_pitchYawRoll.x;
	m_rotation.y += a_pitchYawRoll.y;
	m_rotation.z += a_pitchYawRoll.z;
	m_isMatricesChanged = m_isVectorsChanged = true;
}

void Transform::Scale(float a_x, float a_y, float a_z)
{
	m_scale.x *= a_x;
	m_scale.y *= a_y;
	m_scale.z *= a_z;
	m_isMatricesChanged = true;
}

void Transform::Scale(DirectX::XMFLOAT3 a_scale)
{
	m_scale.x *= a_scale.x;
	m_scale.y *= a_scale.y;
	m_scale.z *= a_scale.z;
	m_isMatricesChanged = true;
}

void Transform::UpdateMatrices()
{
	if (m_isMatricesChanged == false) return;

	XMMATRIX t = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX r = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	XMMATRIX s = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

	XMMATRIX world = s * r * t;
	XMStoreFloat4x4(&m_worldMatrix, world);
	XMStoreFloat4x4(&m_worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(world)));

	m_isMatricesChanged = false;
}

void Transform::UpdateVectors()
{
	if (m_isVectorsChanged == false) return;

	XMVECTOR currentRotation = XMLoadFloat3(&m_rotation);
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYawFromVector(currentRotation);

	XMStoreFloat3(&m_right, XMVector3Rotate(XMVectorSet(1, 0, 0, 0), rotQuat));
	XMStoreFloat3(&m_up, XMVector3Rotate(XMVectorSet(0, 1, 0, 0), rotQuat));
	XMStoreFloat3(&m_forward, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), rotQuat));

	m_isVectorsChanged = false;
}
