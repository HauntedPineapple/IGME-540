#pragma once

#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>
#include <string>

#include "Mesh.h"
#include "Transform.h"
#include "Camera.h"
#include "Material.h"

class Entity
{
public:
	Entity(std::shared_ptr<Mesh> a_pMesh, std::shared_ptr<Material> a_pMaterial, std::string a_entityName = "");

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();
	std::string GetEntityName();
	Transform* GetTransform();

	void SetMaterial(std::shared_ptr<Material> a_pMaterial);
	void SetMesh(std::shared_ptr<Mesh> a_pMesh);

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> a_pContext, std::shared_ptr<Camera> a_pCamera);

private:
	std::shared_ptr<Mesh> m_pMesh;
	std::shared_ptr<Material> m_pMaterial;
	Transform m_transform;
	std::string m_entityName;
};