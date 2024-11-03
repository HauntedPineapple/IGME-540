#include "Entity.h"
using namespace DirectX;

Entity::Entity(std::shared_ptr<Mesh> a_pMesh, std::shared_ptr<Material> a_pMaterial, std::string a_entityName)
	:m_pMesh(a_pMesh),
	m_pMaterial(a_pMaterial),
	m_entityName(a_entityName)
{
	m_transform = Transform();
}

std::shared_ptr<Mesh> Entity::GetMesh() { return m_pMesh; }
std::shared_ptr<Material> Entity::GetMaterial() { return m_pMaterial; }
std::string Entity::GetEntityName() { return m_entityName; }
Transform* Entity::GetTransform() { return &m_transform; }

void Entity::SetMaterial(std::shared_ptr<Material> a_pMaterial) { m_pMaterial = a_pMaterial; }

void Entity::SetMesh(std::shared_ptr<Mesh> a_pMesh) { m_pMesh = a_pMesh; }

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> a_pContext, std::shared_ptr<Camera> a_pCamera)
{
	m_pMaterial->SendDataToShader(&m_transform, a_pCamera);
	m_pMesh->Draw(a_pContext);
}
