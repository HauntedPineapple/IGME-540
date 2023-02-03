#pragma once

#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>

#include "Mesh.h"
#include "Transform.h"

class Entity
{
public:
	Entity(std::shared_ptr<Mesh> mesh);

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer);
private:
	std::shared_ptr<Mesh> m_mesh;
	Transform m_transform;
	DirectX::XMFLOAT4 m_colorTintValue;
};

