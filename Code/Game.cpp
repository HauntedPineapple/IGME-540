#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Helpers.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

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

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	LoadShaders();
	CreateGeometry();
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
	m_pCameras.push_back(std::make_shared<Camera>(XMFLOAT3(0.0f, 0.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), aspectRatio, moveSpeed, rotationSpeed, DirectX::XM_PIDIV4, nearClipDistance, farClipDistance));
	m_pCameras.push_back(std::make_shared<Camera>(XMFLOAT3(-5.0f, 0.0f, -3.0f), XMFLOAT3(0.0f, XMConvertToRadians(90), 0.0f), aspectRatio, moveSpeed, rotationSpeed, DirectX::XM_PIDIV2, nearClipDistance, farClipDistance));
	m_pCameras.push_back(std::make_shared<Camera>(XMFLOAT3(1.7f, 0.3f, 10.5f), XMFLOAT3(0.1f, -0.9f, 0.0f), aspectRatio, moveSpeed, rotationSpeed, (DirectX::XM_PIDIV4 / 2) + DirectX::XM_PIDIV4, nearClipDistance, farClipDistance));
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
	m_pCustomPixelShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"StaticPS.cso").c_str());
	m_pTestPixelShader = std::make_shared<SimplePixelShader>(device, context, FixPath(L"TestPixelShader.cso").c_str());
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
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

	const XMFLOAT4 COLOR = XMFLOAT4(0.44f, 0.34f, 0.48f, 1.0f);

	std::shared_ptr<SimplePixelShader> pShader = m_pTestPixelShader;

	std::shared_ptr<Material> whiteMaterial = std::make_shared<Material>(m_pVertexShader, pShader, C_WHITE, 0.35f);
	std::shared_ptr<Material> redMaterial = std::make_shared<Material>(m_pVertexShader, pShader, C_RED, 0.0f);
	std::shared_ptr<Material> greenMaterial = std::make_shared<Material>(m_pVertexShader, pShader, C_GREEN, 0.25f);
	std::shared_ptr<Material> blueMaterial = std::make_shared<Material>(m_pVertexShader, pShader, C_BLUE, 0.5f);
	std::shared_ptr<Material> cyanMaterial = std::make_shared<Material>(m_pVertexShader, pShader, C_CYAN, 0.75f);
	std::shared_ptr<Material> magentaMaterial = std::make_shared<Material>(m_pVertexShader, pShader, C_MAGENTA, 1.0f);
	std::shared_ptr<Material> yellowMaterial = std::make_shared<Material>(m_pVertexShader, pShader, C_YELLOW, 0.5f);
	std::shared_ptr<Material> blackMaterial = std::make_shared<Material>(m_pVertexShader, pShader, C_BLACK, 0.65f);

	std::shared_ptr<Material> customMaterial = std::make_shared<Material>(m_pVertexShader, m_pCustomPixelShader, C_WHITE);

	std::shared_ptr<Mesh> cubeMesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device);
	std::shared_ptr<Mesh> cylinderMesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), device);
	std::shared_ptr<Mesh> helixMesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), device);
	std::shared_ptr<Mesh> quadMesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str(), device);
	std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device);
	std::shared_ptr<Mesh> torusMesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str(), device);
	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/hylian_shield.obj").c_str(), device);

	std::shared_ptr<Entity> helixEntity = std::make_shared<Entity>(helixMesh, std::make_shared<Material>(m_pVertexShader, pShader, C_WHITE, 0.0f), "Helix");
	std::shared_ptr<Entity> cylinderEntity = std::make_shared<Entity>(cylinderMesh, std::make_shared<Material>(m_pVertexShader, pShader, C_WHITE, 0.25f), "Cylinder");
	std::shared_ptr<Entity> cubeEntity = std::make_shared<Entity>(cubeMesh, std::make_shared<Material>(m_pVertexShader, pShader, C_WHITE, 0.4f), "Cube");
	std::shared_ptr<Entity> entity = std::make_shared<Entity>(mesh, std::make_shared<Material>(m_pVertexShader, pShader, C_WHITE, 0.5f), "Model");
	std::shared_ptr<Entity> sphereEntity = std::make_shared<Entity>(sphereMesh, std::make_shared<Material>(m_pVertexShader, pShader, C_WHITE, 0.75f), "Sphere");
	std::shared_ptr<Entity> torusEntity = std::make_shared<Entity>(torusMesh, std::make_shared<Material>(m_pVertexShader, pShader, C_WHITE, 0.9f), "Torus");
	std::shared_ptr<Entity> quadEntity = std::make_shared<Entity>(quadMesh, std::make_shared<Material>(m_pVertexShader, pShader, C_WHITE, 1.0f), "Quad");

	m_pMeshes.push_back(cubeMesh);
	m_pMeshes.push_back(cylinderMesh);
	m_pMeshes.push_back(helixMesh);
	m_pMeshes.push_back(mesh);
	m_pMeshes.push_back(sphereMesh);
	m_pMeshes.push_back(torusMesh);
	m_pMeshes.push_back(quadMesh);

	m_pEntities.push_back(cubeEntity);
	m_pEntities.push_back(cylinderEntity);
	m_pEntities.push_back(helixEntity);
	m_pEntities.push_back(entity);
	m_pEntities.push_back(sphereEntity);
	m_pEntities.push_back(torusEntity);
	m_pEntities.push_back(quadEntity);

	int space = 3;
	for (int i = 0; i < m_pEntities.size(); i++) {
		Transform* p_entityTransform = m_pEntities[i]->GetTransform();
		p_entityTransform->SetPosition(XMFLOAT3(0.0f, 0.0f, 12.0f));
		if (i < m_pEntities.size() / 2) {
			p_entityTransform->MoveRelative(XMFLOAT3(-space * (i + 1.0f), 0.0f, 0.0f));
		}
		else {
			p_entityTransform->MoveRelative(XMFLOAT3(space * (i - 3.0f), 0.0f, 0.0f));
		}
	}
}

void Game::CreateLights()
{
	//m_ambientLightColor = { 0.06f, 0.2f, 0.25f };
	m_ambientLightColor = { 0.1f, 0.1f, 0.25f };

	Light directionalLightA = {};
	directionalLightA.type = 0;
	directionalLightA.direction = { 1 ,0, 0 };
	directionalLightA.color = { 1, 0, 0 };
	directionalLightA.intensity = 1.0f;

	Light directionalLightB = {};
	directionalLightB.type = 0;
	directionalLightB.direction = { 0, 0, 1 };
	directionalLightB.color = { 0, 0, 1 };
	directionalLightB.intensity = 1.0f;

	Light	directionalLightC = {};
	directionalLightC.type = 0;
	directionalLightC.direction = { 0, 1, 0 };
	directionalLightC.color = { 0, 1, 0 };
	directionalLightC.intensity = 1.0f;

	Light	pointLightA = {};
	pointLightA.type = 1;
	pointLightA.color = { 1, 1, 1 };
	pointLightA.intensity = 1.0f;

	Light	m_pointLightB = {};
	m_pointLightB.type = 1;
	m_pointLightB.color = { 1, 1, 1 };
	m_pointLightB.intensity = 1.0f;

	Light spotLightA = {};
	spotLightA.type = 2;
	spotLightA.color = { 1, 1, 1 };
	spotLightA.intensity = 1.0f;

	m_lights.push_back(directionalLightA);
	m_lights.push_back(directionalLightB);
	m_lights.push_back(directionalLightC);
	//m_pLights.push_back(pointLightA);
	//m_pLights.push_back(std::make_shared<Light>(pointLightB);
	//m_pLights.push_back(spotLightA);
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

	for (int i = 0; i < m_pEntities.size(); i++)
	{
		std::string entityName = m_pEntities[i]->GetEntityName();
		Transform* entityTransform = m_pEntities[i]->GetTransform();
		XMFLOAT3 entityRot = entityTransform->GetRotation();
		XMFLOAT3 entityPos = entityTransform->GetPosition();
		XMFLOAT3 entityScale = entityTransform->GetScale();

		if (entityName == "Model") {

		}
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
				LightsGUI(m_lights[i]);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Camera Controls"))
	{
		CameraGUI();
	}

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

void Game::LightsGUI(Light a_light)
{
	switch (a_light.type) {
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

	if (a_light.type == 0 || a_light.type == 2)
	{
		ImGui::DragFloat3("Direction", &a_light.direction.x, 0.005f, -1, 1);
	}
	if (a_light.type == 1 || a_light.type == 2)
	{
		ImGui::DragFloat("Range", &a_light.range, 0.1f);
		ImGui::DragFloat3("Position", &a_light.position.x, 0.01f);
	}
	if (a_light.type == 2)
	{
		ImGui::DragFloat("Falloff", &a_light.spotFalloff, 0.1f);
	}

	ImGui::DragFloat("Intensity", &a_light.intensity, 0.01f, 0.0f, 1000.0f);
	ImGui::ColorEdit3("Color", (float*)&a_light.color);
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

		pixelShader->SetFloat3("ambientColor", m_ambientLightColor);

		//pixelShader->SetData("directionalLightA", &*m_pLights[0], sizeof(Light));
		//pixelShader->SetData("directionalLightB", &*m_pLights[1], sizeof(Light));
		//pixelShader->SetData("directionalLightC", &*m_pLights[2], sizeof(Light));
		pixelShader->SetData("lights", &m_lights[0], sizeof(Light) * (int)m_lights.size());

		entity->Draw(context, m_pCameras[m_currentCamIndex]);
	}

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