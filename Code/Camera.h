#pragma once

#include <DirectXMath.h>
#include "Transform.h"

class Camera {
public:
	Camera(DirectX::XMFLOAT3 a_initPos, float a_moveSpeed, float a_rotationSpeed, float a_FOV, float a_aspectRatio);

	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();

	void Update(float a_dt);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float a_aspectRatio);

private:
	DirectX::XMFLOAT4X4 m_viewMatrix;
	DirectX::XMFLOAT4X4 m_projectionMatrix;

	Transform m_Transform;

	float m_moveSpeed;
	float m_rotationSpeed;
	float m_fieldOfView;
	//float m_mouseLookSpeed;
	//float m_nearClipDistance;
	//float m_farClipDistance;

	//bool isOrthographic;
};