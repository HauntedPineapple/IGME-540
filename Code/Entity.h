#pragma once

#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>

#include "Mesh.h"
#include "Transform.h"
#include "Camera.h"

class Entity
{
public:
	Entity(std::shared_ptr<Mesh> a_pMesh);

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> a_pContext, Microsoft::WRL::ComPtr<ID3D11Buffer> a_pVsConstantBuffer, std::shared_ptr<Camera> a_pCamera);

private:
	std::shared_ptr<Mesh> m_pMesh;
	Transform m_transform;
};

