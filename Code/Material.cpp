#include "Material.h"

Material::Material(std::shared_ptr<SimpleVertexShader> a_pVertexShader, std::shared_ptr<SimplePixelShader> a_pPixelShader, DirectX::XMFLOAT3 a_colorTint, float a_roughness, bool a_useSpecularMap, DirectX::XMFLOAT2 a_uvScale, DirectX::XMFLOAT2 a_uvOffset)
{
	m_pVertexShader = a_pVertexShader;
	m_pPixelShader = a_pPixelShader;
	m_colorTint = a_colorTint;
	SetRoughness(a_roughness);
	m_useSpecularMap = a_useSpecularMap;
	m_uvScale = a_uvScale;
	m_uvOffset = a_uvOffset;
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return m_pVertexShader; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return m_pPixelShader; }
DirectX::XMFLOAT3 Material::GetColorTint() { return m_colorTint; }
float Material::GetRoughness() { return m_roughness; }
bool Material::GetUseSpecularMap(bool a_useSpecularMap) { return m_useSpecularMap; }
DirectX::XMFLOAT2 Material::GetUVScale() { return m_uvScale; }
DirectX::XMFLOAT2 Material::GetUVOffset() { return m_uvOffset; }

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> a_pVertexShader) { m_pVertexShader = a_pVertexShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> a_pPixelShader) { m_pPixelShader = a_pPixelShader; }
void Material::SetColorTint(DirectX::XMFLOAT3 a_colorTint) { m_colorTint = a_colorTint; }
void Material::SetRoughness(float a_roughness) {
	if (a_roughness < 0.0f) m_roughness = 0.0f;
	if (a_roughness > 1.0f) m_roughness = 1.0f;
	else m_roughness = a_roughness;
}
void Material::SetUVScale(DirectX::XMFLOAT2 a_uvScale) { m_uvScale = a_uvScale; }
void Material::SetUVOffset(DirectX::XMFLOAT2 a_uvOffset) { m_uvOffset = a_uvOffset; }
void Material::AddTextureSRV(std::string a_shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> a_srv) { m_textureSRVs.insert({ a_shaderName, a_srv }); }
void Material::AddSampler(std::string a_shaderName, Microsoft::WRL::ComPtr<ID3D11SamplerState> a_sampler) { m_samplers.insert({ a_shaderName, a_sampler }); }

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
	m_pPixelShader->SetFloat3("colorTint", this->GetColorTint());

	for (auto& t : m_textureSRVs) { m_pPixelShader->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : m_samplers) { m_pPixelShader->SetSamplerState(s.first.c_str(), s.second); }

	m_pPixelShader->SetFloat2("uvScale", m_uvScale);
	m_pPixelShader->SetFloat2("uvOffset", m_uvOffset);
	m_pPixelShader->SetInt("useSpecularMap", (int)m_useSpecularMap);

	m_pPixelShader->CopyAllBufferData();
}
