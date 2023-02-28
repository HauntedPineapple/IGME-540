#pragma once

#include <memory>

#include "SimpleShader.h"
#include "Transform.h"
#include "Camera.h"

class Material
{
public:
	Material(std::shared_ptr<SimpleVertexShader> a_pVertexShader, std::shared_ptr<SimplePixelShader> a_pPixelShader, DirectX::XMFLOAT3 a_colorTint, float a_roughness = 0.0f);

	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	DirectX::XMFLOAT3 GetColorTint();
	float GetRoughness();

	void SetVertexShader(std::shared_ptr<SimpleVertexShader> a_pVertexShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> a_pPixelShader);
	void SetColorTint(DirectX::XMFLOAT3 a_colorTint);
	void SetRoughness(float a_roughness);

	void SendDataToShader(Transform* a_transform, std::shared_ptr<Camera> a_pCamera);

private:
	std::shared_ptr<SimpleVertexShader> m_pVertexShader;
	std::shared_ptr<SimplePixelShader> m_pPixelShader;
	DirectX::XMFLOAT3 m_colorTint;
	float m_roughness;
};
