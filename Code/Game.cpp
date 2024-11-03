#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Helpers.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "WICTextureLoader.h"

#include "string"
#include "cmath"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

	m_ambientLightColor = {};
	m_currentCamIndex = 0;
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs

	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	ImGui::StyleColorsDark();

	// Helper methods for loading and creating stuff
	LoadShaders();
	LoadTextures();
	LoadMeshes();
	CreateEntities();
	CreateLights();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		context->IASetInputLayout(inputLayout.Get());
	}

	// Create our cameras
	m_currentCamIndex = 0;
	float aspectRatio = (float)this->windowWidth / this->windowHeight;
	float moveSpeed = 8.0f;
	float rotationSpeed = 0.005f;
	float nearClipDistance = 0.01f;
	float farClipDistance = 100;
	m_pCameras.push_back(std::make_shared<Camera>(XMFLOAT3(0.0f, 0.0f, -15.0f), XMFLOAT3(0, 0.0f, 0.0f), aspectRatio, moveSpeed, rotationSpeed, DirectX::XM_PIDIV4, nearClipDistance, farClipDistance));
	m_pCameras.push_back(std::make_shared<Camera>(XMFLOAT3(0.0f, 15.0f, -30.0f), XMFLOAT3(0.475f, 0.0f, 0.0f), aspectRatio, moveSpeed, rotationSpeed, 32.0f, nearClipDistance, farClipDistance));
	m_pCameras.push_back(std::make_shared<Camera>(XMFLOAT3(1.7f, 0.3f, 10.5f), XMFLOAT3(0.1f, -0.9f, 0.0f), aspectRatio, moveSpeed, rotationSpeed, (DirectX::XM_PIDIV4 / 2) + DirectX::XM_PIDIV4, nearClipDistance, farClipDistance));

	m_stopEntityMovement = false;

	m_gamma = 2.2f;
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// Since the Direct3D helper functions for loading shaders under the hood require wide characters, 
	// the string literal must be preceded by an L.
	m_pVertexShader = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"VertexShader.cso").c_str());
	m_pPixelShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"PixelShader.cso").c_str());
	m_pSkyVS = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"SkyVS.cso").c_str());
	m_pSkyPS = std::make_shared<SimplePixelShader>(device, context, FixPath(L"SkyPS.cso").c_str());
	m_pPBRShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"PBRPixelShader.cso").c_str());
	m_pTexturePixelShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"TexturePixelShader.cso").c_str());
	//m_pStaticEffectPixelShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"StaticPS.cso").c_str());
}

void Game::LoadTextures()
{
	// Create a sampler state
	D3D11_SAMPLER_DESC samplerStateDescription = {};
	samplerStateDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDescription.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerStateDescription.MaxAnisotropy = 16;
	samplerStateDescription.MaxLOD = D3D11_FLOAT32_MAX; // enable mipmapping at any range
	device->CreateSamplerState(&samplerStateDescription, m_pTextureSampler.GetAddressOf());

	//CreateWICTextureFromFile(
	//  device.Get(),
	//	context.Get(),
	//	FixPath(L"../../Assets/Textures/filename.png").c_str(),
	//	0,
	//	m_pSRV.GetAddressOf());


	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/UV.png").c_str(), 0, m_uvTexture.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/flat_normals.png").c_str(), 0, m_flatNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/normalTestN.png").c_str(), 0, m_normalTestSRV.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/T_HylianShield_BC.png").c_str(), 0, m_shieldDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/T_HylianShield_Specular.png").c_str(), 0, m_shieldSpec.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/T_HylianShield_N.png").c_str(), 0, m_shieldNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/T_HylianShield_Metal.png").c_str(), 0, m_shieldMetal.GetAddressOf());
CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/T_HylianShield_Roughness.png").c_str(), 0, m_shieldRough.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/minecraft/T_Player.png").c_str(), 0, m_minecraftSkinSRV.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rustymetal.png").c_str(), 0, m_rustyMetalDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rustymetal_specular.png").c_str(), 0, m_rustyMetalSpec.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/brokentiles.png").c_str(), 0, m_brokenTilesDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/brokentiles_specular.png").c_str(), 0, m_brokenTilesSpec.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/tiles.png").c_str(), 0, m_tilesDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/tiles_specular.png").c_str(), 0, m_tilesSpec.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/blue_painted_planks_diff.png").c_str(), 0, m_bluePlanksDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/blue_painted_planks_spec.png").c_str(), 0, m_bluePlanksSpec.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/blue_painted_planks_n.png").c_str(), 0, m_bluePlanksNormal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/metal_plate_diff.png").c_str(), 0, m_metalPlateDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/metal_plate_specular.png").c_str(), 0, m_metalPlateSpec.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/metal_plate_n.png").c_str(), 0, m_metalPlateNormal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/stone_tiles_diff.png").c_str(), 0, m_stoneTilesDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/stone_tiles_n.png").c_str(), 0, m_stoneTilesNormal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone.png").c_str(), 0, m_cobblestoneDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cobblestone_normals.png").c_str(), 0, m_cobblestoneNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_metal.png").c_str(), 0, m_cobblestoneMetal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_roughness.png").c_str(), 0, m_cobblestoneRough.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cushion.png").c_str(), 0, m_cushionDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/cushion_normals.png").c_str(), 0, m_cushionNormal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rock.png").c_str(), 0, m_rockDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/rock_normals.png").c_str(), 0, m_rockNormal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/forest_ground_diff.png").c_str(), 0, m_forestGroundDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/forest_ground_n.png").c_str(), 0, m_forestGroundNormal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/bronze_albedo.png").c_str(), 0, m_bronzeDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/bronze_normals.png").c_str(), 0, m_bronzeNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/bronze_metal.png").c_str(), 0, m_bronzeMetal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/bronze_roughness.png").c_str(), 0, m_bronzeRough.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/floor_albedo.png").c_str(), 0, m_floorDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/floor_normals.png").c_str(), 0, m_floorNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/floor_metal.png").c_str(), 0, m_floorMetal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/floor_roughness.png").c_str(), 0, m_floorRough.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/scratched_albedo.png").c_str(), 0, m_scratchedDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/scratched_normals.png").c_str(), 0, m_scratchedNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/scratched_metal.png").c_str(), 0, m_bronzeMetal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/scratched_roughness.png").c_str(), 0, m_scratchedRough.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_albedo.png").c_str(), 0, m_paintDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_normals.png").c_str(), 0, m_paintNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/bronze_metal.png").c_str(), 0, m_paintMetal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_roughness.png").c_str(), 0, m_paintRough.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/rough_albedo.png").c_str(), 0, m_roughDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/rough_normals.png").c_str(), 0, m_roughNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/rough_metal.png").c_str(), 0, m_roughMetal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/rough_roughness.png").c_str(), 0, m_roughRough.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_albedo.png").c_str(), 0, m_woodDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_normals.png").c_str(), 0, m_woodNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_metal.png").c_str(), 0, m_bronzeMetal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_roughness.png").c_str(), 0, m_woodRough.GetAddressOf());
}

void Game::LoadMeshes()
{
	// https://www.geeksforgeeks.org/unordered_map-in-cpp-stl/
	// https://en.cppreference.com/w/cpp/container/unordered_map
	m_pMeshes["cube"] = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device);
	m_pMeshes["cylinder"] = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), device);
	m_pMeshes["helix"] = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), device);
	m_pMeshes["sphere"] = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device);
	m_pMeshes["torus"] = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str(), device);
	m_pMeshes["quad"] = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str(), device);
	m_pMeshes["hylian shield"] = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/hylian_shield.obj").c_str(), device);
	m_pMeshes["minecraft player"] = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/Steve.obj").c_str(), device);

	m_pMeshes["test mesh"] = m_pMeshes["sphere"];
	m_pMeshes["uv mesh"] = m_pMeshes["sphere"];
}

void Game::CreateEntities()
{
#pragma region Colors
	const XMFLOAT3 C_BLACK = XMFLOAT3(0.0f, 0.0f, 0.0f);
	const XMFLOAT3 C_WHITE = XMFLOAT3(1.0f, 1.0f, 1.0f);
	const XMFLOAT3 C_RED = XMFLOAT3(1.0f, 0.0f, 0.0f);
	const XMFLOAT3 C_ORANGE = XMFLOAT3(1.0f, 0.5f, 0.0f);
	const XMFLOAT3 C_YELLOW = XMFLOAT3(1.0f, 1.0f, 0.0f);
	const XMFLOAT3 C_CHARTREUSE = XMFLOAT3(0.5f, 1.0f, 0.0f);
	const XMFLOAT3 C_GREEN = XMFLOAT3(0.0f, 1.0f, 0.0f);
	const XMFLOAT3 C_SPRING = XMFLOAT3(0.0f, 1.0f, 0.5f);
	const XMFLOAT3 C_CYAN = XMFLOAT3(0.0f, 1.0f, 1.0f);
	const XMFLOAT3 C_AZURE = XMFLOAT3(0.0f, 0.5f, 1.0f);
	const XMFLOAT3 C_BLUE = XMFLOAT3(0.0f, 0.0f, 1.0f);
	const XMFLOAT3 C_VIOLET = XMFLOAT3(0.5f, 0.0f, 1.0f);
	const XMFLOAT3 C_PINK = XMFLOAT3(1.0f, 0.0f, 1.0f);
	const XMFLOAT3 C_MAGENTA = XMFLOAT3(1.0f, 0.0f, 0.5f);
#pragma endregion

#pragma region Materials
	std::shared_ptr<Material> whiteMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_WHITE, 1.0f);
	std::shared_ptr<Material> redMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_RED, 0.43f);
	std::shared_ptr<Material> greenMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_GREEN, 0.14f);
	std::shared_ptr<Material> blueMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_BLUE, 0.56f);
	std::shared_ptr<Material> cyanMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_CYAN, 1.0f);
	std::shared_ptr<Material> magentaMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_MAGENTA, 0.74f);
	std::shared_ptr<Material> yellowMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_YELLOW, 0.26f);
	std::shared_ptr<Material> blackMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_BLACK, 1.0f);

	m_pEditableMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 1.0f);
	m_pEditableMaterial->AddTextureSRV("DiffuseTexture", m_uvTexture);
	m_pEditableMaterial->AddTextureSRV("NormalMap", m_flatNormal);
	//m_pEditableMaterial->AddTextureSRV("NormalMap", m_normalTestSRV);
	m_pEditableMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> hylianShieldMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 0.0f, true);
	hylianShieldMaterial->AddTextureSRV("DiffuseTexture", m_shieldDiff);
	hylianShieldMaterial->AddTextureSRV("SpecularMap", m_shieldSpec);
	hylianShieldMaterial->AddTextureSRV("NormalMap", m_shieldNormal);
	hylianShieldMaterial->AddTextureSRV("Metalness", m_shieldMetal);
	hylianShieldMaterial->AddTextureSRV("Roughness", m_shieldRough);
	hylianShieldMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> minecraftPlayerMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 1.0f);
	minecraftPlayerMaterial->AddTextureSRV("DiffuseTexture", m_minecraftSkinSRV);
	minecraftPlayerMaterial->AddTextureSRV("NormalMap", m_flatNormal);
	minecraftPlayerMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> rustyMetalMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 0.0f, true);
	rustyMetalMaterial->AddTextureSRV("DiffuseTexture", m_rustyMetalDiff);
	rustyMetalMaterial->AddTextureSRV("SpecularMap", m_rustyMetalSpec);
	rustyMetalMaterial->AddTextureSRV("NormalMap", m_flatNormal);
	rustyMetalMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> brokenTilesMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 0.0f, true);
	brokenTilesMaterial->AddTextureSRV("DiffuseTexture", m_brokenTilesDiff);
	brokenTilesMaterial->AddTextureSRV("SpecularMap", m_brokenTilesSpec);
	brokenTilesMaterial->AddTextureSRV("NormalMap", m_flatNormal);
	brokenTilesMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> tilesMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 0.0f, true);
	tilesMaterial->AddTextureSRV("DiffuseTexture", m_tilesDiff);
	tilesMaterial->AddTextureSRV("SpecularMap", m_tilesSpec);
	tilesMaterial->AddTextureSRV("NormalMap", m_flatNormal);
	tilesMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> bluePlanksMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 0.0f, true);
	bluePlanksMaterial->AddTextureSRV("DiffuseTexture", m_bluePlanksDiff);
	bluePlanksMaterial->AddTextureSRV("SpecularMap", m_bluePlanksSpec);
	bluePlanksMaterial->AddTextureSRV("NormalMap", m_bluePlanksNormal);
	bluePlanksMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> metalPlateMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 0.0f, true);
	metalPlateMaterial->AddTextureSRV("DiffuseTexture", m_metalPlateDiff);
	metalPlateMaterial->AddTextureSRV("SpecularMap", m_metalPlateSpec);
	metalPlateMaterial->AddTextureSRV("NormalMap", m_metalPlateNormal);
	metalPlateMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> stoneTilesMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 1.0f);
	stoneTilesMaterial->AddTextureSRV("DiffuseTexture", m_stoneTilesDiff);
	stoneTilesMaterial->AddTextureSRV("NormalMap", m_stoneTilesNormal);
	stoneTilesMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> uvMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 1.0f, false);
	uvMaterial->AddTextureSRV("DiffuseTexture", m_uvTexture);
	uvMaterial->AddTextureSRV("NormalMap", m_flatNormal);
	uvMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> cobblestoneMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 1.0f, false);
	cobblestoneMaterial->AddTextureSRV("DiffuseTexture", m_cobblestoneDiff);
	cobblestoneMaterial->AddTextureSRV("NormalMap", m_cobblestoneNormal);
	cobblestoneMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> cushionMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 1.0f, false);
	cushionMaterial->AddTextureSRV("DiffuseTexture", m_cushionDiff);
	cushionMaterial->AddTextureSRV("NormalMap", m_cushionNormal);
	cushionMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> rockMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 1.0f, false);
	rockMaterial->AddTextureSRV("DiffuseTexture", m_rockDiff);
	rockMaterial->AddTextureSRV("NormalMap", m_rockNormal);
	rockMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> forestGroundMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 1.0f, false);
	forestGroundMaterial->AddTextureSRV("DiffuseTexture", m_forestGroundDiff);
	forestGroundMaterial->AddTextureSRV("NormalMap", m_forestGroundNormal);
	forestGroundMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> cobblestonePBRMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 1.0f, false);
	cobblestonePBRMaterial->AddTextureSRV("DiffuseTexture", m_cobblestoneDiff);
	cobblestonePBRMaterial->AddTextureSRV("NormalMap", m_cobblestoneNormal);
	cobblestonePBRMaterial->AddTextureSRV("Metalness", m_cobblestoneMetal);
	cobblestonePBRMaterial->AddTextureSRV("Roughness", m_cobblestoneRough);
	cobblestonePBRMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> bronzePBRMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 1.0f, false);
	bronzePBRMaterial->AddTextureSRV("DiffuseTexture", m_bronzeDiff);
	bronzePBRMaterial->AddTextureSRV("NormalMap", m_bronzeNormal);
	bronzePBRMaterial->AddTextureSRV("Metalness", m_bronzeMetal);
	bronzePBRMaterial->AddTextureSRV("Roughness", m_bronzeRough);
	bronzePBRMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> floorPBRMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 1.0f, false);
	floorPBRMaterial->AddTextureSRV("DiffuseTexture", m_floorDiff);
	floorPBRMaterial->AddTextureSRV("NormalMap", m_floorNormal);
	floorPBRMaterial->AddTextureSRV("Metalness", m_floorMetal);
	floorPBRMaterial->AddTextureSRV("Roughness", m_floorRough);
	floorPBRMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> scratchedPBRMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 1.0f, false);
	scratchedPBRMaterial->AddTextureSRV("DiffuseTexture", m_scratchedDiff);
	scratchedPBRMaterial->AddTextureSRV("NormalMap", m_scratchedNormal);
	scratchedPBRMaterial->AddTextureSRV("Metalness", m_scratchedMetal);
	scratchedPBRMaterial->AddTextureSRV("Roughness", m_scratchedRough);
	scratchedPBRMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> paintPBRMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 1.0f, false);
	paintPBRMaterial->AddTextureSRV("DiffuseTexture", m_paintDiff);
	paintPBRMaterial->AddTextureSRV("NormalMap", m_paintNormal);
	paintPBRMaterial->AddTextureSRV("Metalness", m_paintMetal);
	paintPBRMaterial->AddTextureSRV("Roughness", m_paintRough);
	paintPBRMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> roughPBRMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 1.0f, false);
	roughPBRMaterial->AddTextureSRV("DiffuseTexture", m_roughDiff);
	roughPBRMaterial->AddTextureSRV("NormalMap", m_bronzeNormal);
	roughPBRMaterial->AddTextureSRV("Metalness", m_roughMetal);
	roughPBRMaterial->AddTextureSRV("Roughness", m_roughRough);
	roughPBRMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> woodPBRMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 1.0f, false);
	woodPBRMaterial->AddTextureSRV("DiffuseTexture", m_woodDiff);
	woodPBRMaterial->AddTextureSRV("NormalMap", m_woodNormal);
	woodPBRMaterial->AddTextureSRV("Metalness", m_woodMetal);
	woodPBRMaterial->AddTextureSRV("Roughness", m_woodRough);
	woodPBRMaterial->AddSampler("BasicSampler", m_pTextureSampler);
#pragma endregion

	float meshSpacing = 4;
	int previousSize = 0;
	int currentSize = 0;
	std::vector<std::shared_ptr<Entity>> entityRow;

	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["cube"], blueMaterial, "Cube"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["cylinder"], greenMaterial, "Cylinder"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["helix"], redMaterial, "Helix"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["hylian shield"], hylianShieldMaterial, "Hylian Shield"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["sphere"], cyanMaterial, "Sphere"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["torus"], magentaMaterial, "Torus"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["quad"], yellowMaterial, "Quad"));
	currentSize = (int)m_pEntities.size() - previousSize;
	SetEntitiesInRow(std::vector<std::shared_ptr<Entity>>(m_pEntities.begin() + previousSize, m_pEntities.begin() + currentSize + previousSize),
		XMFLOAT3(0.0f, 0.0f, 10.0f), meshSpacing);
	previousSize = (int)m_pEntities.size();


	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], rustyMetalMaterial, "Texture Test 1"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], tilesMaterial, "Texture Test 2"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], brokenTilesMaterial, "Texture Test 3"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["minecraft player"], minecraftPlayerMaterial, "Minecraft Player"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], bluePlanksMaterial, "Texture Test 4"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], metalPlateMaterial, "Texture Test 5"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], stoneTilesMaterial, "Texture Test 6"));
	currentSize = (int)m_pEntities.size() - previousSize;
	SetEntitiesInRow(std::vector<std::shared_ptr<Entity>>(m_pEntities.begin() + previousSize, m_pEntities.begin() + currentSize + previousSize),
		XMFLOAT3(0.0f, -1.0f, 5.0f), meshSpacing);
	previousSize = (int)m_pEntities.size();

	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], cobblestoneMaterial, "Normal Test 1"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], cushionMaterial, "Normal Test 2"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["uv mesh"], m_pEditableMaterial, "UV Mesh"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], rockMaterial, "Normal Test 3"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], forestGroundMaterial, "Normal Test 4"));
	currentSize = (int)m_pEntities.size() - previousSize;
	SetEntitiesInRow(std::vector<std::shared_ptr<Entity>>(m_pEntities.begin() + previousSize, m_pEntities.begin() + currentSize + previousSize),
		XMFLOAT3(0.0f, -2.0f, 0.0f), meshSpacing);
	previousSize = (int)m_pEntities.size();

	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], bronzePBRMaterial, "PBR Test 1"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], floorPBRMaterial, "PBR Test 2"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], scratchedPBRMaterial, "PBR Test 3"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], cobblestonePBRMaterial, "PBR Test 4"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], paintPBRMaterial, "PBR Test 5"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], roughPBRMaterial, "PBR Test 6"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], woodPBRMaterial, "PBR Test 7"));
	currentSize = (int)m_pEntities.size() - previousSize;
	SetEntitiesInRow(std::vector<std::shared_ptr<Entity>>(m_pEntities.begin() + previousSize, m_pEntities.begin() + currentSize + previousSize),
		XMFLOAT3(0.0f, -3.0f, -5.0f), meshSpacing);
	previousSize = (int)m_pEntities.size();

	CreateSky();
}

void Game::SetEntitiesInRow(std::vector<std::shared_ptr<Entity>> a_pEntities, XMFLOAT3 a_origin, float a_spacing)
{
	int halfway = (int)a_pEntities.size() / 2;
	for (int i = 0; i < a_pEntities.size(); i++) {
		Transform* p_entityTransform = a_pEntities[i]->GetTransform();
		p_entityTransform->SetPosition(a_origin);
		if (i < halfway) {
			p_entityTransform->MoveRelative(XMFLOAT3(-a_spacing * (i + 1), 0.0f, 0.0f));
		}
		else if (i > halfway) {
			p_entityTransform->MoveRelative(XMFLOAT3(a_spacing * (i - halfway), 0.0f, 0.0f));
		}
	}
}

void Game::CreateSky()
{
	D3D11_SAMPLER_DESC samplerStateDescription = {};
	samplerStateDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDescription.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerStateDescription.MaxAnisotropy = 16;
	samplerStateDescription.MaxLOD = D3D11_FLOAT32_MAX; // enable mipmapping at any range
	device->CreateSamplerState(&samplerStateDescription, m_pTextureSampler.GetAddressOf());

	m_pSky = std::make_shared<Sky>(
		m_pMeshes["cube"],
		m_pTextureSampler,
		device,
		context,
		m_pSkyPS,
		m_pSkyVS,
		FixPath(L"../../Assets/Skies/Clouds Blue/right.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds Blue/left.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds Blue/up.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds Blue/down.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds Blue/front.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds Blue/back.png").c_str());
}

void Game::CreateLights()
{
	m_ambientLightColor = { 0.15f, 0.15f, 0.15f };

	Light directionalLightA = {};
	directionalLightA.type = 0;
	directionalLightA.direction = { 1, 0.5f, 0.5f };
	directionalLightA.color = XMFLOAT3(1, 1, 1);
	directionalLightA.intensity = 1.0f;

	Light directionalLightB = {};
	directionalLightB.type = 0;
	directionalLightB.direction = { -0.25f, -1, 0.75f };
	directionalLightB.color = XMFLOAT3(1, 1, 1);
	directionalLightB.intensity = 1.0f;

	Light directionalLightC = {};
	directionalLightC.type = 0;
	directionalLightC.direction = { -1, 1, -0.5f };
	directionalLightC.color = XMFLOAT3(1, 1, 1);
	directionalLightC.intensity = 1.0f;

	//directionalLightA.direction = { 1 ,0, 0 };
	//directionalLightA.color = { 1, 0, 0 };
	//directionalLightB.direction = { 0, -1, 0};
	//directionalLightB.color = { 0, 1, 0 };
	//directionalLightB.intensity = 1.0f;
	//directionalLightC.direction = { 0, 0, 1 };
	//directionalLightC.color = { 0, 0, 1 };

	Light pointLightA = {};
	pointLightA.type = 1;
	pointLightA.position = { -5.0f, 0, 3 };
	pointLightA.color = { 1, 1, 1 };
	pointLightA.intensity = 0.5f;
	pointLightA.range = 10.0f;

	Light pointLightB = {};
	pointLightB.type = 1;
	pointLightB.position = { 5.0f, 0, 3 };
	pointLightB.color = { 1, 1, 1 };
	pointLightB.intensity = 0.5f;
	pointLightB.range = 25.0f;

	m_lights.push_back(directionalLightA);
	m_lights.push_back(directionalLightB);
	m_lights.push_back(directionalLightC);
	//m_lights.push_back(pointLightA);
	//m_lights.push_back(pointLightB);
}

// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
	for (std::shared_ptr<Camera> cam : m_pCameras)
	{  // update the projection of all cameras
		cam->UpdateProjectionMatrix((float)this->windowWidth / this->windowHeight);
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	this->UpdateGUI(deltaTime, totalTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();

	if (m_stopEntityMovement == false) {
		for (int i = 0; i < m_pEntities.size(); i++)
		{
			std::string entityName = m_pEntities[i]->GetEntityName();
			Transform* entityTransform = m_pEntities[i]->GetTransform();
			XMFLOAT3 entityRot = entityTransform->GetRotation();
			XMFLOAT3 entityPos = entityTransform->GetPosition();
			XMFLOAT3 entityScale = entityTransform->GetScale();

			if (entityName == "Helix") {
				entityTransform->SetRotation(0, entityRot.y + deltaTime, 0);
				if (entityRot.y + deltaTime >= DirectX::XMConvertToRadians(360))
					entityTransform->SetRotation(entityRot.x, 0, entityRot.z);
			}
			if (entityName == "Cylinder") {
				entityTransform->SetPosition(entityPos.x, sin(totalTime), entityPos.z);
			}
			if (entityName == "Cube") {
				entityTransform->SetRotation(0, entityRot.y + deltaTime, entityRot.z + deltaTime);
				if (entityRot.y + deltaTime >= DirectX::XMConvertToRadians(360))
					entityTransform->SetRotation(entityRot.x, 0, entityRot.z);
				if (entityRot.z + deltaTime >= DirectX::XMConvertToRadians(360))
					entityTransform->SetRotation(entityRot.x, entityRot.y, 0);
			}
			if (entityName == "Sphere") {
				entityTransform->SetPosition(entityPos.x, sin(totalTime), entityPos.z);
			}
			if (entityName == "Torus") {
				entityTransform->SetRotation(entityRot.x + deltaTime, 0, 0);
				if (entityRot.x + deltaTime >= DirectX::XMConvertToRadians(360))
					entityTransform->SetRotation(0, entityRot.y, entityRot.z);
			}
			if (entityName == "Quad") {
				entityTransform->SetRotation(0, 0, entityRot.z + deltaTime);
				if (entityRot.z + deltaTime >= DirectX::XMConvertToRadians(360))
					entityTransform->SetRotation(entityRot.x, entityRot.y, 0);
			}

			//if (entityName.find("Test") != -1) {
			//	entityTransform->SetRotation(0, entityRot.y - (deltaTime / 8), 0);
			//	if (abs(entityRot.y - deltaTime) >= DirectX::XMConvertToRadians(360))
			//		entityTransform->SetRotation(entityRot.x, 0, entityRot.z);
			//}
		}
	}

	m_pCameras[m_currentCamIndex]->Update(deltaTime);
}

void Game::UpdateGUI(float deltaTime, float totalTime)
{
	// Feed fresh input data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input& input = Input::GetInstance();
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);

	ImGui::Begin("App Interface");

	ImGui::Checkbox("Stop Entity Movement", &m_stopEntityMovement);
	/*ImGui::SliderFloat("Gamma", &m_gamma, 0.1f, 10.0f);*/

	// Test and UV Mesh Shape Changer
	const char* shapes[] = { "sphere", "cylinder", "cube", "helix", "torus", "quad" };
	static int currentTestShape = 0;
	if (ImGui::Combo("Test Mesh Shape", &currentTestShape, shapes, IM_ARRAYSIZE(shapes))) {
		for (int i = 0; i < m_pEntities.size(); i++)
		{
			std::string entityName = m_pEntities[i]->GetEntityName();
			if (entityName.find("Test") != -1) {
				m_pEntities[i]->SetMesh(m_pMeshes[(std::string)shapes[currentTestShape]]);
			}
		}
	}
	static int currentUVShape = 0;
	if (ImGui::Combo("UV Mesh Shape", &currentUVShape, shapes, IM_ARRAYSIZE(shapes))) {
		for (int i = 0; i < m_pEntities.size(); i++)
		{
			std::string entityName = m_pEntities[i]->GetEntityName();
			if (entityName.find("UV Mesh") != -1) {
				m_pEntities[i]->SetMesh(m_pMeshes[(std::string)shapes[currentUVShape]]);
			}
		}
	}

	if (ImGui::CollapsingHeader("App Info"))
	{
		ImGui::Text("Framerate: %f", ImGui::GetIO().Framerate);
		ImGui::Text("Window Dimensions: %i x %i", this->windowWidth, this->windowHeight);
		ImGui::Text("Cursor Position: %f, %f", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
	}

	if (ImGui::CollapsingHeader("Light Controls")) {
		ImGui::ColorEdit3("Ambient Light Color", (float*)&m_ambientLightColor);
		for (int i = 0; i < m_lights.size(); i++)
		{
			std::string label = "Light " + std::to_string(i + 1);
			ImGui::PushID(i);
			if (ImGui::TreeNode(label.data())) {
				LightsGUI(&m_lights[i]);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Material Controls"))
		ScaleMaterialGUI(m_pEditableMaterial);

	if (ImGui::CollapsingHeader("Camera Controls"))
		CameraGUI();

	if (ImGui::CollapsingHeader("Entity Controls"))
	{
		for (int i = 0; i < m_pEntities.size(); i++)
		{
			std::string label = "Entity " + std::to_string(i + 1);
			if (m_pEntities[i]->GetEntityName() != "")
				label = m_pEntities[i]->GetEntityName();
			ImGui::PushID(i);
			if (ImGui::TreeNode(label.data())) {
				EntityGUI(m_pEntities[i]);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}

	ImGui::End();
}

void Game::ScaleMaterialGUI(std::shared_ptr<Material> a_pScalableMaterial)
{
	float roughness = a_pScalableMaterial->GetRoughness();
	DirectX::XMFLOAT2 uvScale = a_pScalableMaterial->GetUVScale();
	DirectX::XMFLOAT2 uvOffset = a_pScalableMaterial->GetUVOffset();
	DirectX::XMFLOAT3 colorTint = a_pScalableMaterial->GetColorTint();

	if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f))
		a_pScalableMaterial->SetRoughness(roughness);
	if (ImGui::DragFloat("U Scale", &uvScale.x, 0.1f))
		a_pScalableMaterial->SetUVScale(uvScale);
	if (ImGui::DragFloat("V Scale", &uvScale.y, 0.1f))
		a_pScalableMaterial->SetUVScale(uvScale);
	if (ImGui::DragFloat2("UV offset", &uvOffset.x, 0.1f))
		a_pScalableMaterial->SetUVOffset(uvOffset);
	if (ImGui::ColorEdit3("Color Tint", (float*)&colorTint))
		a_pScalableMaterial->SetColorTint(colorTint);
}

void Game::LightsGUI(Light* a_pLight)
{
	switch (a_pLight->type) {
	case 0: // Directional
		ImGui::Text("Directional Light");
		break;
	case 1: // Point
		ImGui::Text("Point Light");
		break;
	case 2: // Spot
		ImGui::Text("Spot Light");
		break;
	}

	if (a_pLight->type == 0 || a_pLight->type == 2)
	{
		ImGui::DragFloat3("Direction", &a_pLight->direction.x, 0.005f, -1, 1);
	}
	if (a_pLight->type == 1 || a_pLight->type == 2)
	{
		ImGui::DragFloat("Range", &a_pLight->range, 0.1f, 0.0f, 5000.0f);
		ImGui::DragFloat3("Position", &a_pLight->position.x, 0.01f);
	}
	if (a_pLight->type == 2)
	{
		ImGui::DragFloat("Falloff", &a_pLight->spotFalloff, 0.1f, 0.0f, 5.0f);
	}

	ImGui::DragFloat("Intensity", &a_pLight->intensity, 0.01f, 0.0f, 5000.0f);
	ImGui::ColorEdit3("Color", (float*)&a_pLight->color);
}

void Game::CameraGUI()
{
	ImGui::RadioButton("Camera 1", &m_currentCamIndex, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Camera 2", &m_currentCamIndex, 1);
	ImGui::SameLine();
	ImGui::RadioButton("Camera 3", &m_currentCamIndex, 2);

	std::shared_ptr<Camera> p_camera = m_pCameras[m_currentCamIndex];
	Transform* p_cameraTransform = p_camera->GetTransform();

	ImGui::Text("Camera Info");

	XMFLOAT3 positionVec = p_cameraTransform->GetPosition();
	if (ImGui::DragFloat3("Position", &positionVec.x, 0.01f)) {
		p_cameraTransform->SetPosition(positionVec);
	}
	XMFLOAT3 rotationVec = p_cameraTransform->GetRotation();
	if (ImGui::DragFloat3("Rotation", &rotationVec.x, 0.01f)) {
		p_cameraTransform->SetRotation(rotationVec);
	}

	//float moveSpeed = p_camera->GetMoveSpeed();
	//if (ImGui::DragFloat("Move Speed", &moveSpeed, 1.0f, 1.0f, 100.0f)) {
	//	p_camera->SetRotationSpeed(moveSpeed);
	//}
	//float rotationSpeed = p_camera->GetRotationSpeed();
	//if (ImGui::DragFloat("Rotation Speed", &rotationSpeed, 0.001f, 0.001f, 0.01f)) {
	//	p_camera->SetRotationSpeed(rotationSpeed);
	//}

	float fieldOfView = XMConvertToDegrees(p_camera->GetFieldOfView());
	if (ImGui::DragFloat("FOV", &fieldOfView, 1.0f, 30.0f, 145.0f)) {
		p_camera->SetFieldOfView(XMConvertToRadians(fieldOfView));
	}
	float nearClip = p_camera->GetNearClipDistance();
	if (ImGui::DragFloat("Near Clip", &nearClip, 0.01f, 0.01f, 1.0f)) {
		p_camera->SetNearClipDistance(nearClip);
	}
	float farClip = p_camera->GetFarClipDistance();
	if (ImGui::DragFloat("Far Clip", &farClip, 1.0f, 5.0f, 1000.0f)) {
		p_camera->SetFarClipDistance(farClip);
	}

	//static bool useOrthographic = false;
	//ImGui::Checkbox("Orthographic Projection", &useOrthographic);
	//if (useOrthographic) {
	//	p_currentCamera->SetProjectionType(true);
	//}
	//else {
	//	p_currentCamera->SetProjectionType(false);
	//}
}

void Game::EntityGUI(std::shared_ptr<Entity> a_pEntity)
{
	Transform* p_entityTransform = a_pEntity->GetTransform();

	XMFLOAT3 positionVec = p_entityTransform->GetPosition();
	if (ImGui::DragFloat3("Position", &positionVec.x, 0.01f)) {
		p_entityTransform->SetPosition(positionVec);
	}

	XMFLOAT3 scaleVec = p_entityTransform->GetScale();
	if (ImGui::DragFloat3("Scale", &scaleVec.x, 0.01f)) {
		p_entityTransform->SetScale(scaleVec);
	}

	XMFLOAT3 rotationVec = p_entityTransform->GetRotation();
	if (ImGui::DragFloat3("Rotation", &rotationVec.x, 0.01f)) {
		p_entityTransform->SetRotation(rotationVec);
	}

	ImGui::Text("Mesh Index Count: %d", a_pEntity->GetMesh()->GetIndexCount());
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry
	for (std::shared_ptr<Entity> entity : m_pEntities) {
		std::shared_ptr<SimplePixelShader> pixelShader = entity->GetMaterial()->GetPixelShader();
		pixelShader->SetFloat("time", totalTime);
		pixelShader->SetFloat("gamma", m_gamma);

		pixelShader->SetFloat3("ambientColor", m_ambientLightColor);

		//pixelShader->SetData("directionalLightA", &*m_pLights[0], sizeof(Light));
		//pixelShader->SetData("directionalLightB", &*m_pLights[1], sizeof(Light));
		//pixelShader->SetData("directionalLightC", &*m_pLights[2], sizeof(Light));
		pixelShader->SetData("lights", &m_lights[0], sizeof(Light) * (int)m_lights.size());

		entity->Draw(context, m_pCameras[m_currentCamIndex]);
	}

	m_pSky->Draw(m_pCameras[m_currentCamIndex]);

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;

		// Draw GUI - must be BEFORE swapchain
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}
}