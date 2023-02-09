#pragma once

#include <DirectXMath.h>
#include "Transform.h"

class Camera {
public:
	/* FOV must be in radians */
	Camera(DirectX::XMFLOAT3 a_initPos, float a_aspectRatio, float a_moveSpeed = 5.0f, float a_rotationSpeed = 0.001f, float a_fieldOfView = DirectX::XM_PIDIV4, float a_nearClipDistance = 0.1f, float a_farClipDistance = 100, bool a_isOrthographic = false);

	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();

	void Update(float a_deltaTime);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float a_aspectRatio);

private:
	Transform m_transform;

	DirectX::XMFLOAT4X4 m_viewMatrix;
	DirectX::XMFLOAT4X4 m_projectionMatrix;

	float m_moveSpeed;
	float m_rotationSpeed;
	float m_aspectRatio;
	float m_fieldOfView; //radians
	float m_nearClipDistance;
	float m_farClipDistance;

	bool m_isOrthographic;
};