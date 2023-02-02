#include "Transform.h"

Transform::Transform()
{
}

DirectX::XMFLOAT3 Transform::GetPosition() { return position; }
DirectX::XMFLOAT3 Transform::GetScale() { return scale; }

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	// Create the full, new world matrix from the 3 transformations
	XMMATRIX t = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX r = XMMatrixRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
	XMMATRIX s = XMMatrixScaling(scale.x, scale.y, scale.z);

	XMMATRIX world = s * r * t;
	XMStoreFloat4x4(&worldMatrix, world);

	return worldMatrix;
}

void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}
void Transform::SetPosition(DirectX::XMFLOAT3 newPosition) { position = newPosition; }

void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	position.x += offset.x;
	position.y += offset.y;
	position.z += offset.z;
}
