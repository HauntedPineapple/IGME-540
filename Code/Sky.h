#pragma once
#include <memory>
#include <wrl/client.h>

#include "Mesh.h"
#include "Camera.h"
#include "SimpleShader.h"

class Sky
{
public:
	Sky();
	~Sky();

	void Draw(std::shared_ptr<Camera> a_camera);
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* a_right,
		const wchar_t* a_left,
		const wchar_t* a_up,
		const wchar_t* a_down,
		const wchar_t* a_front,
		const wchar_t* a_back);

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerOptions;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubemapSRV;
	Microsoft::WRL::ComPtr<ID3D10DepthStencilState> m_depthState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
	std::shared_ptr<Mesh> m_skymesh;
	std::shared_ptr<SimplePixelShader> m_skyPS;
	std::shared_ptr<SimpleVertexShader> m_skyVS;
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
};