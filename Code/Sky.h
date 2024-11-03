#pragma once
#include <memory>
#include <string>
#include <wrl/client.h>

#include "Mesh.h"
#include "Camera.h"
#include "SimpleShader.h"

class Sky
{
public:
	Sky( // Create a sky by passing in the filepaths to the six images for the cubemap
		std::shared_ptr<Mesh> a_pSkyMesh,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> a_pSamplerOptions,
		Microsoft::WRL::ComPtr<ID3D11Device> a_pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> a_pContext,
		std::shared_ptr<SimplePixelShader> a_pSkyPS,
		std::shared_ptr<SimpleVertexShader> a_pSkyVS,
		const wchar_t* a_right,
		const wchar_t* a_left,
		const wchar_t* a_up,
		const wchar_t* a_down,
		const wchar_t* a_front,
		const wchar_t* a_back);

	Sky(
		std::shared_ptr<Mesh> a_pSkyMesh,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> a_pSamplerOptions,
		Microsoft::WRL::ComPtr<ID3D11Device> a_pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> a_pContext,
		std::shared_ptr<SimplePixelShader> a_pSkyPS,
		std::shared_ptr<SimpleVertexShader> a_pSkyVS,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> a_pCubeMapSRV);

	~Sky();

	void Draw(std::shared_ptr<Camera> a_pCamera);

	std::shared_ptr<Mesh> GetSkyMesh();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetCubeMap();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();

	void SetSkyMesh(std::shared_ptr<Mesh> a_pSkyMesh);
	void SetCubeMap(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> a_pCubeMapSRV);
	void SetCubeMap(const wchar_t* a_right, const wchar_t* a_left, const wchar_t* a_up, const wchar_t* a_down, const wchar_t* a_front, const wchar_t* a_back);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> a_pSkyPS);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> a_pSkyVS);
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(const wchar_t* a_right, const wchar_t* a_left, const wchar_t* a_up, const wchar_t* a_down, const wchar_t* a_front, const wchar_t* a_back);

	std::shared_ptr<Mesh> m_pSkyMesh;
	std::shared_ptr<SimplePixelShader> m_pSkyPS;
	std::shared_ptr<SimpleVertexShader> m_pSkyVS;
	Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pCubeMapSRV;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerOptions;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pDepthState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pRasterizerState;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext;
};