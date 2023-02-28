#include "Material.h"

Material::Material(std::shared_ptr<SimpleVertexShader> a_pVertexShader, std::shared_ptr<SimplePixelShader> a_pPixelShader, DirectX::XMFLOAT4 a_colorTint, float a_roughness)
{
	m_pVertexShader = a_pVertexShader;
	m_pPixelShader = a_pPixelShader;
	m_colorTint = a_colorTint;
	SetRoughness(a_roughness);
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return m_pVertexShader; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return m_pPixelShader; }
DirectX::XMFLOAT4 Material::GetColorTint() { return m_colorTint; }
float Material::GetRoughness() { return m_roughness; }

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> a_pVertexShader) { m_pVertexShader = a_pVertexShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> a_pPixelShader) { m_pPixelShader = a_pPixelShader; }
void Material::SetColorTint(DirectX::XMFLOAT4 a_colorTint) { m_colorTint = a_colorTint; }

void Material::SetRoughness(float a_roughness) {
	if (a_roughness < 0.0f) m_roughness = 0.0f;
	if (a_roughness > 1.0f) m_roughness = 1.0f;
	else m_roughness = a_roughness;
}

void Material::SendDataToShader(Transform* a_transform, std::shared_ptr<Camera> a_pCamera)
{
	// Activate shaders
	this->GetVertexShader()->SetShader();
	this->GetPixelShader()->SetShader();

	m_pVertexShader->SetMatrix4x4("worldMatrix", a_transform->GetWorldMatrix());
	m_pVertexShader->SetMatrix4x4("worldInvTransposeMatrix", a_transform->GetWorldInverseTransposeMatrix());
	m_pVertexShader->SetMatrix4x4("viewMatrix", a_pCamera->GetViewMatrix());
	m_pVertexShader->SetMatrix4x4("projectionMatrix", a_pCamera->GetProjectionMatrix());
	m_pVertexShader->CopyAllBufferData();

	m_pPixelShader->SetFloat("roughness", this->GetRoughness());
	m_pPixelShader->SetFloat3("cameraPosition", a_pCamera->GetTransform()->GetPosition());
	m_pPixelShader->SetFloat4("colorTint", this->GetColorTint());
	m_pPixelShader->CopyAllBufferData();
}
