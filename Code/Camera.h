#pragma once

#include <DirectXMath.h>

#include "Transform.h"

class Camera {
public:
	/* FOV must be in radians */
	Camera(
		DirectX::XMFLOAT3 a_initPosition = DirectX::XMFLOAT3(0.0f, 0.0f, -5.0f), 
		DirectX::XMFLOAT3 a_initRotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		float a_aspectRatio = 1.78f, float a_moveSpeed = 5.0f,
		float a_rotationSpeed = 0.003f, float a_fieldOfView = DirectX::XM_PIDIV4,
		float a_nearClipDistance = 0.01f, float a_farClipDistance = 100.0f,
		bool a_isOrthographic = false);

	void Update(float a_deltaTime);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float a_aspectRatio);

	Transform* GetTransform();
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	float GetAspectRatio();
	float GetMoveSpeed();
	float GetRotationSpeed();
	float GetFieldOfView();
	float GetNearClipDistance();
	float GetFarClipDistance();
	// If true: orthographic, if false: perpsective
	bool GetProjectionType();

	void SetAspectRatio(float a_aspectRatio);
	void SetMoveSpeed(float a_moveSpeed);
	void SetRotationSpeed(float a_rotationSpeed);
	void SetFieldOfView(float a_fieldOfView);
	void SetNearClipDistance(float a_nearClipDistance);
	void SetFarClipDistance(float a_farClipDistance);
	// If true: orthographic, if false: perpsective
	void SetProjectionType(bool a_isOrthographic);
private:
	Transform m_transform;

	DirectX::XMFLOAT4X4 m_viewMatrix;
	DirectX::XMFLOAT4X4 m_projectionMatrix;

	float m_aspectRatio;
	float m_moveSpeed;
	float m_rotationSpeed;
	float m_fieldOfView; //radians
	float m_nearClipDistance;
	float m_farClipDistance;

	bool m_isOrthographic;
};