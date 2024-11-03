#pragma once

#include <memory>
#include <unordered_map>

#include "SimpleShader.h"
#include "Transform.h"
#include "Camera.h"

class Material
{
public:
	Material(std::shared_ptr<SimpleVertexShader> a_pVertexShader, std::shared_ptr<SimplePixelShader> a_pPixelShader, DirectX::XMFLOAT3 a_colorTint, float a_roughness = 0.0f, bool a_useSpecularMap = false, DirectX::XMFLOAT2 a_uvScale = DirectX::XMFLOAT2(1, 1), DirectX::XMFLOAT2 a_uvOffset = DirectX::XMFLOAT2(0, 0));

	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	DirectX::XMFLOAT3 GetColorTint();
	float GetRoughness();
	bool GetUseSpecularMap(bool a_useSpecularMap);
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();

	void SetVertexShader(std::shared_ptr<SimpleVertexShader> a_pVertexShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> a_pPixelShader);
	void SetColorTint(DirectX::XMFLOAT3 a_colorTint);
	void SetRoughness(float a_roughness);
	void SetUVScale(DirectX::XMFLOAT2 a_uvScale);
	void SetUVOffset(DirectX::XMFLOAT2 a_uvOffset);

	void AddTextureSRV(std::string a_shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> a_srv);
	void AddSampler(std::string a_shaderName, Microsoft::WRL::ComPtr<ID3D11SamplerState> a_sampler);

	void SendDataToShader(Transform* a_transform, std::shared_ptr<Camera> a_pCamera);

private:
	std::shared_ptr<SimpleVertexShader> m_pVertexShader;
	std::shared_ptr<SimplePixelShader> m_pPixelShader;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> m_samplers;

	DirectX::XMFLOAT3 m_colorTint;
	DirectX::XMFLOAT2 m_uvScale;
	DirectX::XMFLOAT2 m_uvOffset;
	float m_roughness;
	bool m_useSpecularMap;
};
