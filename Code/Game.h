#pragma once

#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>
#include <vector>
#include <unordered_map>

#include "DXCore.h"
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include "Material.h"
#include "SimpleShader.h"
#include "Lights.h"

class Game : public DXCore
{
public:
	Game(HINSTANCE hInstance);
	~Game();

	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);

	void UpdateGUI(float deltaTime, float totalTime);
	void LightsGUI(Light* a_pLight);
	void CameraGUI();
	void EntityGUI(std::shared_ptr<Entity> a_pEntity);

	void Draw(float deltaTime, float totalTime);

private:
	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void LoadTextures();
	void LoadMeshesAndCreateEntities();
	void CreateLights();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	int m_currentCamIndex;

	DirectX::XMFLOAT3 m_ambientLightColor;

	std::vector<Light> m_lights;
	std::vector<std::shared_ptr<Mesh>> m_pMeshes;
	std::vector<std::shared_ptr<Camera>> m_pCameras;
	std::vector<std::shared_ptr<Entity>> m_pEntities;

	std::shared_ptr<SimpleVertexShader> m_pVertexShader;
	std::shared_ptr<SimplePixelShader> m_pPixelShader;
	std::shared_ptr<SimplePixelShader> m_pCustomPixelShader;
	std::shared_ptr<SimplePixelShader> m_pTexturePixelShader;
	std::shared_ptr<SimplePixelShader> m_pTestPixelShader;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pTextureSampler;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pTextureSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pShieldDiffuseSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pShieldSpecularSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pShieldORMSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pDiffuseSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSpecularSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pORMSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pNormalSRV;
};