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
#include "Sky.h"

class Game : public DXCore
{
public:
	Game(HINSTANCE hInstance);
	~Game();

	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);

	void UpdateGUI(float deltaTime, float totalTime);
	void ScaleMaterialGUI(std::shared_ptr<Material> a_pScalableMaterial);
	void LightsGUI(Light* a_pLight);
	void CameraGUI();
	void EntityGUI(std::shared_ptr<Entity> a_pEntity);
	//void SkyGUI();
	//void TextureGUI(std::shared_ptr<Material> a_pMaterial);

	void Draw(float deltaTime, float totalTime);

private:
	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void LoadTextures();
	void LoadMeshes();
	void CreateEntities();
	void SetEntitiesInRow(std::vector<std::shared_ptr<Entity>> a_pEntities, DirectX::XMFLOAT3 a_origin, float a_spacing);
	void CreateSky();
	void CreateLights();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	std::shared_ptr <Sky> m_pSky;
	int m_currentCamIndex;
	float m_gamma;

	DirectX::XMFLOAT3 m_ambientLightColor;
	std::vector<Light> m_lights;
	std::vector<std::shared_ptr<Camera>> m_pCameras;
	std::vector<std::shared_ptr<Entity>> m_pEntities;

	std::shared_ptr<SimpleVertexShader> m_pVertexShader;
	std::shared_ptr<SimpleVertexShader> m_pSkyVS;
	std::shared_ptr<SimplePixelShader> m_pPixelShader;
	std::shared_ptr<SimplePixelShader> m_pSkyPS;
	std::shared_ptr<SimplePixelShader> m_pTexturePixelShader;
	std::shared_ptr<SimplePixelShader> m_pPBRShader;
	//std::shared_ptr<SimplePixelShader> m_pStaticEffectPixelShader;
	//std::shared_ptr<SimplePixelShader> m_pTestPixelShader;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pTextureSampler;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pSkyRasterState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pSkyDepthState;

	std::shared_ptr<Material> m_pEditableMaterial;

	std::unordered_map<std::string, std::shared_ptr<Mesh>> m_pMeshes;
	std::unordered_map<std::string, std::shared_ptr<Material>> m_pMaterials;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_pTextureSRVs;

	bool m_stopEntityMovement;
#pragma region SRVs
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_uvTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_flatNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_normalTestSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_minecraftSkinSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shieldDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shieldSpec;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shieldNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shieldMetal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shieldRough;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_rustyMetalDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_rustyMetalSpec;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_brokenTilesDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_brokenTilesSpec;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_tilesDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_tilesSpec;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bluePlanksDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bluePlanksSpec;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bluePlanksORM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bluePlanksNormal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_metalPlateDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_metalPlateSpec;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_metalPlateORM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_metalPlateNormal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_stoneTilesDiff;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_stoneTilesORM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_stoneTilesNormal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cobblestoneDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cobblestoneNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cobblestoneMetal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cobblestoneRough;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cushionDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cushionNormal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_rockDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_rockNormal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_forestGroundDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_forestGroundNormal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bronzeDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bronzeNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bronzeMetal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bronzeRough;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_floorDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_floorNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_floorMetal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_floorRough;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_scratchedDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_scratchedNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_scratchedMetal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_scratchedRough;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_paintDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_paintNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_paintMetal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_paintRough;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_roughDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_roughNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_roughMetal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_roughRough;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_woodDiff;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_woodNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_woodMetal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_woodRough;
#pragma endregion
};