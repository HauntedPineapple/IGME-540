#include "Camera.h"
#include "Input.h"

using namespace DirectX;

Camera::Camera(DirectX::XMFLOAT3 a_initPosition, DirectX::XMFLOAT3 a_initRotation, float a_aspectRatio, float a_moveSpeed, float a_rotationSpeed, float a_fieldOfView, float a_nearClipDistance, float a_farClipDistance, bool a_isOrthographic) :
	m_aspectRatio(a_aspectRatio),
	m_moveSpeed(a_moveSpeed),
	m_rotationSpeed(a_rotationSpeed),
	m_fieldOfView(a_fieldOfView),
	m_nearClipDistance(a_nearClipDistance),
	m_farClipDistance(a_farClipDistance),
	m_isOrthographic(a_isOrthographic)
{
	m_transform.SetPosition(a_initPosition);
	m_transform.SetRotation(a_initRotation);
	UpdateViewMatrix();
	UpdateProjectionMatrix(a_aspectRatio);
}

void Camera::Update(float a_deltaTime)
{
	Input& input = Input::GetInstance();

	// Scale movement by delta time
	if (input.KeyDown('W')) m_transform.MoveRelative(0, 0, m_moveSpeed * a_deltaTime);
	if (input.KeyDown('A')) m_transform.MoveRelative(-m_moveSpeed * a_deltaTime, 0, 0);
	if (input.KeyDown('S')) m_transform.MoveRelative(0, 0, -m_moveSpeed * a_deltaTime);
	if (input.KeyDown('D')) m_transform.MoveRelative(m_moveSpeed * a_deltaTime, 0, 0);
	if (input.KeyDown(' ')) m_transform.MoveAbsolute(0, m_moveSpeed * a_deltaTime, 0);
	if (input.KeyDown('X')) m_transform.MoveAbsolute(0, -m_moveSpeed * a_deltaTime, 0);

	//if (input.KeyDown(VK_SHIFT)) { /* Shift is down */ }
	//if (input.KeyDown(VK_CONTROL)) { /* Control is down */ }

	// Check for mouse movement when dragging
	if (input.MouseLeftDown())
	{
		float xDiff = input.GetMouseXDelta() * m_rotationSpeed;
		float yDiff = input.GetMouseYDelta() * m_rotationSpeed;
		m_transform.Rotate(yDiff, xDiff, 0);

		if (m_transform.GetRotation().x > XM_2PI) {
			m_transform.SetRotation(XM_2PI, m_transform.GetRotation().y, m_transform.GetRotation().z);
		}
		if (m_transform.GetRotation().x < -XM_2PI) {
			m_transform.SetRotation(-XM_2PI, m_transform.GetRotation().y, m_transform.GetRotation().z);
		}
	}

	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	// Grab the position from the transform
	XMFLOAT3 pos = m_transform.GetPosition();
	XMVECTOR posVector = XMLoadFloat3(&pos);
	// Grab the rotation
	XMFLOAT3 rot = m_transform.GetRotation();
	XMVECTOR rotVector = XMLoadFloat3(&rot);
	// Make a quaternion
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYawFromVector(rotVector);

	XMVECTOR defaultForward = XMVectorSet(0, 0, 1, 0); // z-axis
	XMVECTOR currentForward = XMVector3Rotate(defaultForward, rotQuat);

	XMMATRIX view = XMMatrixLookToLH(posVector, currentForward, XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&m_viewMatrix, view);
}

void Camera::UpdateProjectionMatrix(float a_aspectRatio)
{
	if (m_isOrthographic) {
		// XMStoreFloat4x4(&m_projectionMatrix, XMMatrixOrthographicLH(ViewWidth, ViewHeight, m_nearClipDistance, m_farClipDistance));
	}
	else {
		XMStoreFloat4x4(&m_projectionMatrix, XMMatrixPerspectiveFovLH(m_fieldOfView, a_aspectRatio, m_nearClipDistance, m_farClipDistance));
	}
}

// ================ GETTERS ================
Transform* Camera::GetTransform() { return &m_transform; }
DirectX::XMFLOAT4X4 Camera::GetViewMatrix() { return m_viewMatrix; }
DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix() { return m_projectionMatrix; }
float Camera::GetAspectRatio() { return m_aspectRatio; }
float Camera::GetMoveSpeed() { return m_moveSpeed; }
float Camera::GetRotationSpeed() { return m_rotationSpeed; }
float Camera::GetFieldOfView() { return m_fieldOfView; }
float Camera::GetNearClipDistance() { return m_nearClipDistance; }
float Camera::GetFarClipDistance() { return m_farClipDistance; }
bool Camera::GetProjectionType() { return m_isOrthographic; }

// ================ SETTERS ================
void Camera::SetAspectRatio(float a_aspectRatio)
{
	m_aspectRatio = a_aspectRatio;
	UpdateProjectionMatrix(m_aspectRatio);
}

void Camera::SetMoveSpeed(float a_moveSpeed)
{
	m_moveSpeed = a_moveSpeed;
}

void Camera::SetRotationSpeed(float a_rotationSpeed)
{
	m_rotationSpeed = a_rotationSpeed;
}

void Camera::SetFieldOfView(float a_fieldOfView)
{
	m_fieldOfView = a_fieldOfView;
	UpdateProjectionMatrix(m_aspectRatio);
}

void Camera::SetNearClipDistance(float a_nearClipDistance)
{
	if (a_nearClipDistance < 0 || a_nearClipDistance == m_farClipDistance) return;
	m_nearClipDistance = a_nearClipDistance;
	UpdateProjectionMatrix(m_aspectRatio);
}

void Camera::SetFarClipDistance(float a_farClipDistance)
{
	if (a_farClipDistance < 0 || a_farClipDistance == m_nearClipDistance) return;
	m_farClipDistance = a_farClipDistance;
	UpdateProjectionMatrix(m_aspectRatio);
}

void Camera::SetProjectionType(bool a_isOrthographic)
{
	m_isOrthographic = a_isOrthographic;
	UpdateProjectionMatrix(m_aspectRatio);
}
