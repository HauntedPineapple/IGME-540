#include "Camera.h"
#include "Input.h"

using namespace DirectX;

Camera::Camera(DirectX::XMFLOAT3 a_initPos, float a_moveSpeed, float a_rotationSpeed, float a_FOV, float a_aspectRatio) :
	m_moveSpeed(a_moveSpeed),
	m_rotationSpeed(a_rotationSpeed),
	m_fieldOfView(a_FOV)
{
	m_Transform.SetPosition(a_initPos);
	UpdateViewMatrix();
	UpdateProjectionMatrix(a_aspectRatio);
}

DirectX::XMFLOAT4X4 Camera::GetView() { return m_viewMatrix; }
DirectX::XMFLOAT4X4 Camera::GetProjection() { return m_projectionMatrix; }

void Camera::Update(float a_dt)
{
	Input& input = Input::GetInstance();

	if (input.KeyDown('W')) m_Transform.MoveAbsolute(0, 0, m_moveSpeed * a_dt);
	if (input.KeyDown('A')) m_Transform.MoveAbsolute(-m_moveSpeed * a_dt, 0, 0);
	if (input.KeyDown('S')) m_Transform.MoveAbsolute(0, 0, -m_moveSpeed * a_dt);
	if (input.KeyDown('D')) m_Transform.MoveAbsolute(m_moveSpeed * a_dt, 0, 0);

	// Check for mouse movement when dragging
	if (input.MouseLeftDown())
	{
		float xDiff = input.GetMouseXDelta() * m_rotationSpeed;
		float yDiff = input.GetMouseYDelta() * m_rotationSpeed;
		m_Transform.Rotate(yDiff, xDiff, 0);
	}

	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	// Grab the position from the transform
	XMFLOAT3 pos = m_Transform.GetPosition();
	XMVECTOR posVector = XMLoadFloat3(&pos);

	// Grab the rotation
	XMFLOAT3 rot = m_Transform.GetRotation();
	XMVECTOR rotVector = XMLoadFloat3(&rot);
	// Make a quaternion
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYawFromVector(rotVector);

	XMVECTOR defaultForward = XMVectorSet(0, 0, 1, 0);
	XMVECTOR currentForward = XMVector3Rotate(defaultForward, rotQuat);

	XMMATRIX view = XMMatrixLookToLH(posVector, currentForward, XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&m_viewMatrix, view);
}

void Camera::UpdateProjectionMatrix(float a_aspectRatio)
{
	XMMATRIX proj = XMMatrixPerspectiveFovLH(m_fieldOfView, a_aspectRatio, 0.01f, 1000.0f);
	XMStoreFloat4x4(&m_projectionMatrix, proj);
}
