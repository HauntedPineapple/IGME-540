#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Helpers.h"
#include "BufferStructs.h"

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
	// Pick a style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();

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

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		context->VSSetShader(vertexShader.Get(), 0, 0);
		context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	// The byte width of our buffer needs to be a multiple of 16.
	// It also needs to be greater than or equal to the size(in bytes)
	// of the struct we defined to match our vertex shader's constant buffer
	unsigned int size = sizeof(VertexShaderExternalData);// Get size as the next multiple of 16 (instead of hardcoding a size here!)
	// Adding 15 ensures that we either go past the next multiple of 16, 
	// or if size is already a multiple, we almost get to the next multiple.
	size = (size + 15) / 16 * 16; // This will work even if the struct size changes

	// Describe the constant buffer
	D3D11_BUFFER_DESC cbDesc = {}; // Sets struct to all zeros
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = size; // Must be a multiple of 16
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;

	device->CreateBuffer(&cbDesc, 0, m_pVsConstantBuffer.GetAddressOf());

	// Bind the constant buffer to the right place
	context->VSSetConstantBuffers(
		0, // Which slot (register) to bind the buffer to?
		1, // How many are we activating? Can do multiple at once
		m_pVsConstantBuffer.GetAddressOf()); // Array of buffers (or the address of one)

	m_currentCamIndex = 0;
	// Create our cameras
	float aspectRatio = (float)this->windowWidth / this->windowHeight;
	float moveSpeed = 5.0f;
	float rotationSpeed = 0.005f;
	float nearClipDistance = 0.01f;
	float farClipDistance = 100;
	m_pCameras.push_back(std::make_shared<Camera>(XMFLOAT3(0.0f, 0.0f, -3.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),aspectRatio, moveSpeed, rotationSpeed, DirectX::XM_PIDIV4, nearClipDistance, farClipDistance));
	m_pCameras.push_back(std::make_shared<Camera>(XMFLOAT3(-5.0f, 0.0f, -3.0f), XMFLOAT3(0.0f, XMConvertToRadians(90), 0.0f), aspectRatio, moveSpeed, rotationSpeed,DirectX::XM_PIDIV2, nearClipDistance, farClipDistance));
	m_pCameras.push_back(std::make_shared<Camera>(XMFLOAT3(2.0f, 2.0f, -2.0f), XMFLOAT3(0.5f, -0.8f, 0.0f),aspectRatio, moveSpeed, rotationSpeed, (DirectX::XM_PIDIV4 / 2) + DirectX::XM_PIDIV4, nearClipDistance, farClipDistance));
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
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 orange = XMFLOAT4(0.9f, 0.6f, 0.0f, 1.0f);
	XMFLOAT4 purple = XMFLOAT4(0.5f, 0.0f, 1.0f, 0.8f);
	XMFLOAT4 yellow = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	Vertex triangleVertices[] =
	{
		{ XMFLOAT3(+0.35f, +0.75f, +0.0f), red },
		{ XMFLOAT3(+0.55f, +0.25f, +0.0f), blue },
		{ XMFLOAT3(+0.25f, +0.25f, +0.0f), green },
	};
	// Set up indices, which tell us which vertices to use and in which order
	unsigned int triangleIndices[] = { 0, 1, 2 };
	// Create mesh object
	m_pMeshes.push_back(std::make_shared<Mesh>(triangleVertices,
		sizeof(triangleVertices) / sizeof(triangleVertices[0]),
		triangleIndices,
		sizeof(triangleIndices) / sizeof(triangleIndices[0]),
		device, context));

	Vertex quadVertices[] =
	{
		{ XMFLOAT3(-0.8f, +0.8f, +0.0f), white },
		{ XMFLOAT3(-0.5f, +0.8f, +0.0f), black },
		{ XMFLOAT3(-0.5f, +0.5f, +0.0f), black },
		{ XMFLOAT3(-0.8f, +0.5f, +0.0f), white },
	};
	unsigned int quadIndices[] = {
									0, 1, 2,
									0, 2, 3
	};
	m_pMeshes.push_back(std::make_shared<Mesh>(quadVertices,
		sizeof(quadVertices) / sizeof(quadVertices[0]),
		quadIndices,
		sizeof(quadIndices) /
		sizeof(quadIndices[0]), device, context));

	Vertex hexaVerts[] =
	{
		{ XMFLOAT3(+0.0f, +0.0f, +0.0f), white},
		{ XMFLOAT3(+0.15f, -0.15f, +0.0f), green},
		{ XMFLOAT3(+0.15f, -0.45f, +0.0f), red},
		{ XMFLOAT3(+0.0f, -0.6f, +0.0f), blue},
		{ XMFLOAT3(-0.15f, -0.45f, +0.0f), black},
		{ XMFLOAT3(-0.15f, -0.15f, +0.0f), purple},
	};
	unsigned int hexaIndices[] = {
									0, 1, 5,
									5, 1, 2,
									5, 2, 4,
									4, 2, 3
	};
	std::shared_ptr<Mesh> hexMesh = std::make_shared<Mesh>(hexaVerts,
		sizeof(hexaVerts) / sizeof(hexaVerts[0]),
		hexaIndices,
		sizeof(hexaIndices) / sizeof(hexaIndices[0]),
		device, context);
	m_pMeshes.push_back(hexMesh);

	for (std::shared_ptr<Mesh> mesh : m_pMeshes) {
		m_pEntities.push_back(std::make_shared<Entity>(mesh));
	}

	// Make a bunch more hexagon entities that share the same mesh and move them
	std::shared_ptr<Entity> extraHex1 = std::make_shared<Entity>(hexMesh);
	extraHex1->GetTransform()->Scale(0.25f, 0.25f, 0.0f);
	extraHex1->GetTransform()->MoveAbsolute(0.6f, 0.1f, 0.0f);
	m_pEntities.push_back(extraHex1);
	std::shared_ptr<Entity> extraHex2 = std::make_shared<Entity>(hexMesh);
	extraHex2->GetTransform()->Scale(0.5f, 0.5f, 0.0f);
	extraHex2->GetTransform()->MoveAbsolute(-0.5f, -0.5f, 0.0f);
	m_pEntities.push_back(extraHex2);
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
	m_pCameras[m_currentCamIndex]->UpdateProjectionMatrix(this->windowWidth / this->windowHeight);
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

	// Move stuff around
	float sinTime = sin(totalTime);
	m_pEntities[0]->GetTransform()->SetPosition(sinTime, 0, 0);
	float cosTime = cos(totalTime);
	m_pEntities[1]->GetTransform()->SetPosition(0, cosTime, 0);
	float scale = abs(sinTime);
	m_pEntities[2]->GetTransform()->SetScale(scale, scale, scale);
	m_pEntities[3]->GetTransform()->SetRotation(0, 0, scale * 3);

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

	// Create UI
	ImGui::Begin("App Interface"); // create the window with given name

	if (ImGui::CollapsingHeader("App Info"))
	{
		ImGui::Text("Framerate: %f", ImGui::GetIO().Framerate);
		ImGui::Text("Window Dimensions: %i x %i", this->windowWidth, this->windowHeight);
		ImGui::Text("Cursor Position: %f, %f", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
	}

	if (ImGui::CollapsingHeader("Camera Controls"))
	{
		CameraGUI();
	}

	if (ImGui::CollapsingHeader("Entity Controls"))
	{ // TODO: Make this more compact later
		if (ImGui::TreeNode("Entity 1"))
		{
			EntityGUI(m_pEntities[0]);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Entity 2"))
		{
			EntityGUI(m_pEntities[1]);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Entity 3"))
		{
			EntityGUI(m_pEntities[2]);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Entity 4"))
		{
			EntityGUI(m_pEntities[3]);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Entity 5"))
		{
			EntityGUI(m_pEntities[4]);
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

void Game::CameraGUI()
{
	ImGui::RadioButton("Camera 1", &m_currentCamIndex, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Camera 2", &m_currentCamIndex, 1);
	ImGui::SameLine();
	ImGui::RadioButton("Camera 3", &m_currentCamIndex, 2);

	std::shared_ptr<Camera> p_currentCamera = m_pCameras[m_currentCamIndex];
	Transform* p_cameraTransform = p_currentCamera->GetTransform();

	ImGui::Text("Camera Info");

	XMFLOAT3 positionVec = p_cameraTransform->GetPosition();
	if (ImGui::DragFloat3("Position", &positionVec.x, 0.01f)) {
		p_cameraTransform->SetPosition(positionVec);
	}
	XMFLOAT3 rotationVec = p_cameraTransform->GetRotation();
	if (ImGui::DragFloat3("Rotation", &rotationVec.x, 0.01f)) {
		p_cameraTransform->SetRotation(rotationVec);
	}

	//p_currentCamera->GetProjectionType();
	static bool show_another_window = false;
	ImGui::Checkbox("Orthographic Projection", &show_another_window);
	if(show_another_window)ImGui::Text("TRUE");
	else ImGui::Text("FALSE");
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
		entity->Draw(context, m_pVsConstantBuffer, m_pCameras[m_currentCamIndex]);
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