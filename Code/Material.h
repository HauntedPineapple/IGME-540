#pragma once

#include <memory>

#include "SimpleShader.h"

class Material
{
public:
	Material(std::shared_ptr<SimpleVertexShader> a_pVertexShader, std::shared_ptr<SimplePixelShader> a_pPixelShader, DirectX::XMFLOAT4 a_colorTint);

	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	DirectX::XMFLOAT4 GetColorTint();

	void SetVertexShader(std::shared_ptr<SimpleVertexShader> a_pVertexShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> a_pPixelShader);
	void SetColorTint(DirectX::XMFLOAT4 a_colorTint);

private:
	std::shared_ptr<SimpleVertexShader> m_pVertexShader;
	std::shared_ptr<SimplePixelShader> m_pPixelShader;
	DirectX::XMFLOAT4 m_colorTint;
};
