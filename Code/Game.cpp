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
	CreateShadowResources();

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

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/minecraft/T_Player.png").c_str(), 0, m_minecraftSkinSRV.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/T_HylianShield_BC.png").c_str(), 0, m_shieldDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/T_HylianShield_Specular.png").c_str(), 0, m_shieldSpec.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/T_HylianShield_N.png").c_str(), 0, m_shieldNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/T_HylianShield_Metal.png").c_str(), 0, m_shieldMetal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/T_HylianShield_Roughness.png").c_str(), 0, m_shieldRough.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/FarronDagger_High_Diffuse.png").c_str(), 0, m_daggerDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/FarronDagger_High_Normal.png").c_str(), 0, m_daggerNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/FarronDagger_High_Metal.png").c_str(), 0, m_daggerMetal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/FarronDagger_High_Roughness.png").c_str(), 0, m_daggerRough.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/artorias-sword_diffuse.png").c_str(), 0, m_swordDiff.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/artorias-sword_normal.png").c_str(), 0, m_swordNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/artorias-sword_metal.png").c_str(), 0, m_swordMetal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/model_textures/artorias-sword_roughness.png").c_str(), 0, m_swordRough.GetAddressOf());

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
	m_pMeshes["minecraft player"] = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/Steve.obj").c_str(), device);
	m_pMeshes["hylian shield"] = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/hylian_shield.obj").c_str(), device);
	m_pMeshes["dagger"] = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/farron-dagger-highpoly.obj").c_str(), device);
	m_pMeshes["sword"] = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sword-of-artorias.obj").c_str(), device);

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
	std::shared_ptr<Material> blackMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_BLACK, 1.0f);
	std::shared_ptr<Material> whiteMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_WHITE, 1.0f);
	std::shared_ptr<Material> redMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_RED, 0.43f);
	std::shared_ptr<Material> orangeMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_ORANGE, 0.43f);
	std::shared_ptr<Material> yellowMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_YELLOW, 0.43f);
	std::shared_ptr<Material> chartreuseMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_CHARTREUSE, 0.43f);
	std::shared_ptr<Material> greenMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_GREEN, 0.14f);
	std::shared_ptr<Material> springMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_SPRING, 0.56f);
	std::shared_ptr<Material> cyanMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_CYAN, 1.0f);
	std::shared_ptr<Material> azureMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_AZURE, 1.0f);
	std::shared_ptr<Material> blueMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_BLUE, 0.56f);
	std::shared_ptr<Material> violetMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_VIOLET, 0.56f);
	std::shared_ptr<Material> pinkMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_PINK, 0.26f);
	std::shared_ptr<Material> magentaMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, C_MAGENTA, 0.74f);

	m_pEditableMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 1.0f);
	m_pEditableMaterial->AddTextureSRV("DiffuseTexture", m_uvTexture);
	m_pEditableMaterial->AddTextureSRV("NormalMap", m_flatNormal);
	//m_pEditableMaterial->AddTextureSRV("NormalMap", m_normalTestSRV);
	m_pEditableMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> minecraftPlayerMaterial = std::make_shared<Material>(m_pVertexShader, m_pTexturePixelShader, C_WHITE, 1.0f);
	minecraftPlayerMaterial->AddTextureSRV("DiffuseTexture", m_minecraftSkinSRV);
	minecraftPlayerMaterial->AddTextureSRV("NormalMap", m_flatNormal);
	minecraftPlayerMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> hylianShieldMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 0.0f, true);
	hylianShieldMaterial->AddTextureSRV("DiffuseTexture", m_shieldDiff);
	hylianShieldMaterial->AddTextureSRV("SpecularMap", m_shieldSpec);
	hylianShieldMaterial->AddTextureSRV("NormalMap", m_shieldNormal);
	hylianShieldMaterial->AddTextureSRV("MetalMap", m_shieldMetal);
	hylianShieldMaterial->AddTextureSRV("RoughnessMap", m_shieldRough);
	hylianShieldMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> daggerMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 0.0f, true);
	daggerMaterial->AddTextureSRV("DiffuseTexture", m_daggerDiff);
	daggerMaterial->AddTextureSRV("NormalMap", m_daggerNormal);
	daggerMaterial->AddTextureSRV("MetalMap", m_daggerMetal);
	daggerMaterial->AddTextureSRV("RoughnessMap", m_daggerRough);
	daggerMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> swordMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 0.0f, true);
	swordMaterial->AddTextureSRV("DiffuseTexture", m_swordDiff);
	swordMaterial->AddTextureSRV("NormalMap", m_swordNormal);
	swordMaterial->AddTextureSRV("MetalMap", m_swordMetal);
	swordMaterial->AddTextureSRV("RoughnessMap", m_swordRough);
	swordMaterial->AddSampler("BasicSampler", m_pTextureSampler);

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
	cobblestonePBRMaterial->AddTextureSRV("MetalMap", m_cobblestoneMetal);
	cobblestonePBRMaterial->AddTextureSRV("RoughnessMap", m_cobblestoneRough);
	cobblestonePBRMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> bronzePBRMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 1.0f, false);
	bronzePBRMaterial->AddTextureSRV("DiffuseTexture", m_bronzeDiff);
	bronzePBRMaterial->AddTextureSRV("NormalMap", m_bronzeNormal);
	bronzePBRMaterial->AddTextureSRV("MetalMap", m_bronzeMetal);
	bronzePBRMaterial->AddTextureSRV("RoughnessMap", m_bronzeRough);
	bronzePBRMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> floorPBRMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 1.0f, false);
	floorPBRMaterial->AddTextureSRV("DiffuseTexture", m_floorDiff);
	floorPBRMaterial->AddTextureSRV("NormalMap", m_floorNormal);
	floorPBRMaterial->AddTextureSRV("MetalMap", m_floorMetal);
	floorPBRMaterial->AddTextureSRV("RoughnessMap", m_floorRough);
	floorPBRMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> scratchedPBRMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 1.0f, false);
	scratchedPBRMaterial->AddTextureSRV("DiffuseTexture", m_scratchedDiff);
	scratchedPBRMaterial->AddTextureSRV("NormalMap", m_scratchedNormal);
	scratchedPBRMaterial->AddTextureSRV("MetalMap", m_scratchedMetal);
	scratchedPBRMaterial->AddTextureSRV("RoughnessMap", m_scratchedRough);
	scratchedPBRMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> paintPBRMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 1.0f, false);
	paintPBRMaterial->AddTextureSRV("DiffuseTexture", m_paintDiff);
	paintPBRMaterial->AddTextureSRV("NormalMap", m_paintNormal);
	paintPBRMaterial->AddTextureSRV("MetalMap", m_paintMetal);
	paintPBRMaterial->AddTextureSRV("RoughnessMap", m_paintRough);
	paintPBRMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> roughPBRMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 1.0f, false);
	roughPBRMaterial->AddTextureSRV("DiffuseTexture", m_roughDiff);
	roughPBRMaterial->AddTextureSRV("NormalMap", m_bronzeNormal);
	roughPBRMaterial->AddTextureSRV("MetalMap", m_roughMetal);
	roughPBRMaterial->AddTextureSRV("RoughnessMap", m_roughRough);
	roughPBRMaterial->AddSampler("BasicSampler", m_pTextureSampler);

	std::shared_ptr<Material> woodPBRMaterial = std::make_shared<Material>(m_pVertexShader, m_pPBRShader, C_WHITE, 1.0f, false);
	woodPBRMaterial->AddTextureSRV("DiffuseTexture", m_woodDiff);
	woodPBRMaterial->AddTextureSRV("NormalMap", m_woodNormal);
	woodPBRMaterial->AddTextureSRV("MetalMap", m_woodMetal);
	woodPBRMaterial->AddTextureSRV("RoughnessMap", m_woodRough);
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
		XMFLOAT3(0.0f, 1.0f, 20.0f), meshSpacing);
	previousSize = (int)m_pEntities.size();

	// Texture Tests
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], rustyMetalMaterial, "Texture Test 1"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], tilesMaterial, "Texture Test 2"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], brokenTilesMaterial, "Texture Test 3"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["minecraft player"], minecraftPlayerMaterial, "Minecraft Player"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], bluePlanksMaterial, "Texture Test 4"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], metalPlateMaterial, "Texture Test 5"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], stoneTilesMaterial, "Texture Test 6"));
	currentSize = (int)m_pEntities.size() - previousSize;
	SetEntitiesInRow(std::vector<std::shared_ptr<Entity>>(m_pEntities.begin() + previousSize, m_pEntities.begin() + currentSize + previousSize),
		XMFLOAT3(0.0f, 0.0f, 15.0f), meshSpacing);
	previousSize = (int)m_pEntities.size();

	// Normal Tests
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], cobblestoneMaterial, "Normal Test 1"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], cushionMaterial, "Normal Test 2"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["dagger"], daggerMaterial, "Farron Dagger"));
	Transform* daggerTransform = m_pEntities[(int)m_pEntities.size() - 1]->GetTransform();
	daggerTransform->SetScale({ 2.0f, 2.0f, 2.0f });
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["uv mesh"], m_pEditableMaterial, "UV Mesh"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], rockMaterial, "Normal Test 3"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], forestGroundMaterial, "Normal Test 4"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["sword"], swordMaterial, "Sword of Artorias"));
	Transform* swordTransform = m_pEntities[(int)m_pEntities.size() - 1]->GetTransform();
	swordTransform->SetScale({ 3.0f, 3.0f, 3.0f });
	currentSize = (int)m_pEntities.size() - previousSize;
	SetEntitiesInRow(std::vector<std::shared_ptr<Entity>>(m_pEntities.begin() + previousSize, m_pEntities.begin() + currentSize + previousSize),
		XMFLOAT3(0.0f, -1.0f, 10.0f), meshSpacing);
	previousSize = (int)m_pEntities.size();

	// PBR Tests
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], bronzePBRMaterial, "PBR Test 1"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], floorPBRMaterial, "PBR Test 2"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], scratchedPBRMaterial, "PBR Test 3"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], cobblestonePBRMaterial, "PBR Test 4"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], paintPBRMaterial, "PBR Test 5"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], roughPBRMaterial, "PBR Test 6"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["test mesh"], woodPBRMaterial, "PBR Test 7"));
	currentSize = (int)m_pEntities.size() - previousSize;
	SetEntitiesInRow(std::vector<std::shared_ptr<Entity>>(m_pEntities.begin() + previousSize, m_pEntities.begin() + currentSize + previousSize),
		XMFLOAT3(0.0f, -2.0f, 5.0f), meshSpacing);
	previousSize = (int)m_pEntities.size();

	// Shadow Tests
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["cube"], chartreuseMaterial, "Shadow Caster 1"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["cylinder"], violetMaterial, "Shadow Caster 2"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["helix"], pinkMaterial, "Shadow Caster 3"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["torus"], azureMaterial, "Shadow Caster 4"));
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["sphere"], orangeMaterial, "Shadow Caster 5"));
	currentSize = (int)m_pEntities.size() - previousSize;
	SetEntitiesInRow(std::vector<std::shared_ptr<Entity>>(m_pEntities.begin() + previousSize, m_pEntities.begin() + currentSize + previousSize),
		XMFLOAT3(0.0f, -3.0f, 0.0f), meshSpacing);
	previousSize = (int)m_pEntities.size();

	// Create floor
	m_pEntities.push_back(std::make_shared<Entity>(m_pMeshes["cube"],
		std::make_shared<Material>(m_pVertexShader, m_pPixelShader, XMFLOAT3(0.95f, 0.85f, 0.69f), 1.0f),
		"Floor"));
	Transform* floorTransform = m_pEntities[(int)m_pEntities.size() - 1]->GetTransform();
	floorTransform->SetScale({ 25.0f, 0.25f, 25.0f });
	floorTransform->MoveRelative({ 0.0f, -4.0f, 0.0f });

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
	directionalLightA.direction = { 0.4f, -1.0f, 0.65f };
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

void Game::CreateShadowResources() {
	m_shadowMapResolution = 1024;

	// Create the shadow map texture
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = m_shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.Height = m_shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the associated views (DSV and SRV)
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture.Get(), &shadowDSDesc, m_shadowDSV.GetAddressOf());
	/*
	* ViewDimension describes the dimensionality of the resource. In this case it’s just a 2D texture
	* MipSlice tells the depth view which mip map to render into. We only have 1, so it’s index 0
	* MipLevels and MostDetailedMip tell the SRV which mips it can read. Again, we only have 1.
	* Format is slightly different for each view (D32_FLOAT vs. R32_FLOAT). These both mean “treat
	* all 32 bits as a single value”, but D32_FLOAT is specific to depth views.
	*/
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc = {};
	shadowSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowSRVDesc.Texture2D.MipLevels = 1;
	shadowSRVDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture.Get(), &shadowSRVDesc, m_shadowSRV.GetAddressOf());

	// Create the light/shadow view matrices: 
	// To render from the light’s point of view, we’ll need view and projection matrices that match the light.
	XMMATRIX lightView = XMMatrixLookToLH(
		XMVectorSet(-m_lights[0].direction.x * 20, -m_lights[0].direction.y * 20, -m_lights[0].direction.z * 20, 0),
		XMVectorSet(m_lights[0].direction.x, m_lights[0].direction.y, m_lights[0].direction.z, 0),
		XMVectorSet(0, 1, 0, 0)
	);
	XMStoreFloat4x4(&m_shadowViewMatrix, lightView);

	m_lightProjectionSize = 50.0f;
	XMMATRIX lightProjection = XMMatrixOrthographicLH(
		m_lightProjectionSize,
		m_lightProjectionSize,
		1.0f, 300.0f);
	XMStoreFloat4x4(&m_shadowProjectionMatrix, lightProjection);

	// Create the rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	device->CreateRasterizerState(&shadowRastDesc, &m_shadowRasterizer);

	// Create the sampler for comparison
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	device->CreateSamplerState(&shadowSampDesc, &m_shadowSampler);
}

void Game::RenderToShadowMap() {
	// Clear the shadow map
	context->ClearDepthStencilView(m_shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(m_shadowRasterizer.Get());

	// Set up the output merger stage
	ID3D11RenderTargetView* nullRTV{};
	context->OMSetRenderTargets(1, &nullRTV, m_shadowDSV.Get());

	// Deactivate pixel shader
	context->PSSetShader(0, 0, 0);

	// Change viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)m_shadowMapResolution;
	viewport.Height = (float)m_shadowMapResolution;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	// Entity render loop
	m_pShadowVS = std::make_shared<SimpleVertexShader>(device, context, FixPath(L"ShadowVS.cso").c_str());
	m_pShadowVS->SetShader();
	m_pShadowVS->SetMatrix4x4("view", m_shadowViewMatrix);
	m_pShadowVS->SetMatrix4x4("projection", m_shadowProjectionMatrix);
	for (auto& e : m_pEntities)
	{
		m_pShadowVS->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		m_pShadowVS->CopyAllBufferData();
		// Draw the mesh directly to avoid the entity's material
		e->GetMesh()->Draw(context);
	}

	// Reset the pipeline
	viewport.Width = (float)this->windowWidth;
	viewport.Height = (float)this->windowHeight;
	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	context->RSSetState(0);
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

			if (entityName.find("Shadow") != -1) {
				entityTransform->SetPosition(entityPos.x, sin(totalTime) - 3.0f, entityPos.z);
				entityTransform->SetRotation(entityRot.x + (deltaTime / 8), 0, 0);
				if (entityRot.x + (deltaTime / 8) >= DirectX::XMConvertToRadians(360))
					entityTransform->SetRotation(0, entityRot.y, entityRot.z);
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
	ImGui::Image(m_shadowSRV.Get(), ImVec2(50, 50));
	ImGui::Checkbox("Stop Entity Movement", &m_stopEntityMovement);
	ImGui::SliderFloat("Gamma", &m_gamma, 0.0f, 10.0f);

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

	if (ImGui::SliderFloat("RoughnessMap", &roughness, 0.0f, 1.0f))
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
		XMMATRIX shView = XMMatrixLookToLH(
			XMVectorSet(-m_lights[0].direction.x * 20, -m_lights[0].direction.y * 20, -m_lights[0].direction.z * 20, 0),
			XMVectorSet(m_lights[0].direction.x, m_lights[0].direction.y, m_lights[0].direction.z, 0),
			XMVectorSet(0, 1, 0, 0)
		);
		XMStoreFloat4x4(&m_shadowViewMatrix, shView);
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

	RenderToShadowMap();

	// DRAW geometry
	for (std::shared_ptr<Entity> entity : m_pEntities) {
		std::shared_ptr<SimpleVertexShader> vertexShader = entity->GetMaterial()->GetVertexShader();
		vertexShader->SetMatrix4x4("lightView", m_shadowViewMatrix);
		vertexShader->SetMatrix4x4("lightProjection", m_shadowProjectionMatrix);

		std::shared_ptr<SimplePixelShader> pixelShader = entity->GetMaterial()->GetPixelShader();
		pixelShader->SetFloat("time", totalTime);
		pixelShader->SetFloat("gamma", m_gamma);

		pixelShader->SetFloat3("ambientColor", m_ambientLightColor);

		//pixelShader->SetData("directionalLightA", &*m_pLights[0], sizeof(Light));
		//pixelShader->SetData("directionalLightB", &*m_pLights[1], sizeof(Light));
		//pixelShader->SetData("directionalLightC", &*m_pLights[2], sizeof(Light));
		pixelShader->SetData("lights", &m_lights[0], sizeof(Light) * (int)m_lights.size());

		pixelShader->SetShaderResourceView("ShadowMap", m_shadowSRV);
		pixelShader->SetSamplerState("ShadowSampler", m_shadowSampler);

		entity->Draw(context, m_pCameras[m_currentCamIndex]);
	}

	m_pSky->Draw(m_pCameras[m_currentCamIndex]);

	ID3D11ShaderResourceView* nullSRVs[128] = {};
	context->PSSetShaderResources(0, 128, nullSRVs);

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