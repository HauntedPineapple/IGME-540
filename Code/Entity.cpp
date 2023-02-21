#include "Entity.h"
using namespace DirectX;

Entity::Entity(std::shared_ptr<Mesh> a_pMesh, std::shared_ptr<Material> a_pMaterial)
	:m_pMesh(a_pMesh),
	m_pMaterial(a_pMaterial)
{
	m_transform = Transform();
}

std::shared_ptr<Mesh> Entity::GetMesh() { return m_pMesh; }
std::shared_ptr<Material> Entity::GetMaterial() { return m_pMaterial; }
Transform* Entity::GetTransform() { return &m_transform; }

void Entity::SetMaterial(std::shared_ptr<Material> a_pMaterial) { m_pMaterial = a_pMaterial; }

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> a_pContext, std::shared_ptr<Camera> a_pCamera)
{
	// Activate shaders
	m_pMaterial->GetVertexShader()->SetShader();
	m_pMaterial->GetPixelShader()->SetShader();


	// Create local data for the constant buffer struct
	std::shared_ptr<SimpleVertexShader> vsData = m_pMaterial->GetVertexShader();
	vsData->SetMatrix4x4("worldMatrix", m_transform.GetWorldMatrix());
	vsData->SetMatrix4x4("viewMatrix", a_pCamera->GetViewMatrix());
	vsData->SetMatrix4x4("projectionMatrix", a_pCamera->GetProjectionMatrix());
	vsData->CopyAllBufferData();

	std::shared_ptr<SimplePixelShader> psData = m_pMaterial->GetPixelShader();
	psData->SetFloat4("colorTint", m_pMaterial->GetColorTint());
	psData->CopyAllBufferData();

	m_pMesh->Draw(a_pContext);
}
