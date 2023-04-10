#include "Sky.h"
#include "WICTextureLoader.h"

using namespace DirectX;

Sky::Sky(
	std::shared_ptr<Mesh> a_pSkyMesh, 
	Microsoft::WRL::ComPtr<ID3D11SamplerState> a_pSamplerOptions, 
	Microsoft::WRL::ComPtr<ID3D11Device> a_pDevice, 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> a_pContext, 
	std::shared_ptr<SimplePixelShader> a_pSkyPS, 
	std::shared_ptr<SimpleVertexShader> a_pSkyVS, 
	const wchar_t* a_right, const wchar_t* a_left, const wchar_t* a_up, const wchar_t* a_down, const wchar_t* a_front, const wchar_t* a_back)
{
	m_pSkyMesh = a_pSkyMesh;
	m_pSamplerOptions = a_pSamplerOptions;
	m_pDevice = a_pDevice;
	m_pContext = a_pContext;
	m_pSkyPS = a_pSkyPS;
	m_pSkyVS = a_pSkyVS;
	m_pCubeMapSRV = CreateCubemap(a_right, a_left, a_up, a_down, a_front, a_back);

	// Create render and rasterizer states
	D3D11_RASTERIZER_DESC rasterizerDescription = {};
	rasterizerDescription.FillMode = D3D11_FILL_SOLID;
	rasterizerDescription.CullMode = D3D11_CULL_FRONT;
	rasterizerDescription.DepthClipEnable = true;
	m_pDevice->CreateRasterizerState(&rasterizerDescription, m_pRasterizerState.GetAddressOf());
	D3D11_DEPTH_STENCIL_DESC depthDescription = {};
	depthDescription.DepthEnable = true;
	depthDescription.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	m_pDevice->CreateDepthStencilState(&depthDescription, m_pDepthState.GetAddressOf());
}

Sky::Sky(std::shared_ptr<Mesh> a_pSkyMesh, Microsoft::WRL::ComPtr<ID3D11SamplerState> a_pSamplerOptions, Microsoft::WRL::ComPtr<ID3D11Device> a_pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> a_pContext, std::shared_ptr<SimplePixelShader> a_pSkyPS, std::shared_ptr<SimpleVertexShader> a_pSkyVS, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> a_pCubeMapSRV)
{
	m_pSkyMesh = a_pSkyMesh;
	m_pSamplerOptions = a_pSamplerOptions;
	m_pDevice = a_pDevice;
	m_pSkyPS = a_pSkyPS;
	m_pCubeMapSRV = a_pCubeMapSRV;
}

Sky::~Sky() {}

void Sky::Draw(std::shared_ptr<Camera> a_pCamera)
{
	m_pContext->RSSetState(m_pRasterizerState.Get());
	m_pContext->OMSetDepthStencilState(m_pDepthState.Get(), 0);

	m_pSkyVS->SetShader();
	m_pSkyVS->SetMatrix4x4("viewMatrix", a_pCamera->GetViewMatrix());
	m_pSkyVS->SetMatrix4x4("projectionMatrix", a_pCamera->GetProjectionMatrix());
	m_pSkyVS->CopyAllBufferData();

	m_pSkyPS->SetShader();
	m_pSkyPS->SetShaderResourceView("CubeMap", m_pCubeMapSRV);
	m_pSkyPS->SetSamplerState("BasicSampler", m_pSamplerOptions);

	m_pContext->RSSetState(0); // Null (or 0) puts back the defaults
	m_pContext->OMSetDepthStencilState(0, 0);
}

std::shared_ptr<Mesh> Sky::GetSkyMesh() { return m_pSkyMesh; }
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Sky::GetCubeMap() { return m_pCubeMapSRV; }
std::shared_ptr<SimplePixelShader> Sky::GetPixelShader() { return m_pSkyPS; }std::shared_ptr<SimpleVertexShader> Sky::GetVertexShader() { return m_pSkyVS; }

void Sky::SetSkyMesh(std::shared_ptr<Mesh> a_pSkyMesh)
{
}

void Sky::SetCubeMap(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> a_pCubeMapSRV)
{
}

void Sky::SetCubeMap(const wchar_t* a_right, const wchar_t* a_left, const wchar_t* a_up, const wchar_t* a_down, const wchar_t* a_front, const wchar_t* a_back)
{
}

void Sky::SetPixelShader(std::shared_ptr<SimplePixelShader> a_pSkyPS)
{
}

void Sky::SetVertexShader(std::shared_ptr<SimpleVertexShader> a_pSkyVS)
{
}

// --------------------------------------------------------
// Author: Chris Cascioli
// Purpose: Loads six individual textures (the six faces of a cube map), then
// creates a blank cube map and copies each of the six textures to
// another face.  Afterwards, creates a shader resource view for
// the cube map and cleans up all of the temporary resources.
// --------------------------------------------------------
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Sky::CreateCubemap(const wchar_t* a_right, const wchar_t* a_left, const wchar_t* a_up, const wchar_t* a_down, const wchar_t* a_front, const wchar_t* a_back)
{
	// Load the 6 textures into an array.
		// - We need references to the TEXTURES, not SHADER RESOURCE VIEWS!
		// - Explicitly NOT generating mipmaps, as we don't need them for the sky!
		// - Order matters here!  +X, -X, +Y, -Y, +Z, -Z
	Microsoft::WRL::ComPtr<ID3D11Texture2D> textures[6] = {};

	CreateWICTextureFromFile(m_pDevice.Get(), a_right, (ID3D11Resource**)textures[0].GetAddressOf(), 0);
	CreateWICTextureFromFile(m_pDevice.Get(), a_left, (ID3D11Resource**)textures[1].GetAddressOf(), 0);
	CreateWICTextureFromFile(m_pDevice.Get(), a_up, (ID3D11Resource**)textures[2].GetAddressOf(), 0);
	CreateWICTextureFromFile(m_pDevice.Get(), a_down, (ID3D11Resource**)textures[3].GetAddressOf(), 0);
	CreateWICTextureFromFile(m_pDevice.Get(), a_front, (ID3D11Resource**)textures[4].GetAddressOf(), 0);
	CreateWICTextureFromFile(m_pDevice.Get(), a_back, (ID3D11Resource**)textures[5].GetAddressOf(), 0);

	// We'll assume all of the textures are the same color format and resolution,
	// so get the description of the first texture
	D3D11_TEXTURE2D_DESC faceDesc = {};
	textures[0]->GetDesc(&faceDesc);

	// Describe the resource for the cube map, which is simply 
	// a "texture 2d array" with the TEXTURECUBE flag set.  
	// This is a special GPU resource format, NOT just a 
	// C++ array of textures!!!
	D3D11_TEXTURE2D_DESC cubeDesc = {};
	cubeDesc.ArraySize = 6;            // Cube map!
	cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // We'll be using as a texture in a shader
	cubeDesc.CPUAccessFlags = 0;       // No read back
	cubeDesc.Format = faceDesc.Format; // Match the loaded texture's color format
	cubeDesc.Width = faceDesc.Width;   // Match the size
	cubeDesc.Height = faceDesc.Height; // Match the size
	cubeDesc.MipLevels = 1;            // Only need 1
	cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // This should be treated as a CUBE, not 6 separate textures
	cubeDesc.Usage = D3D11_USAGE_DEFAULT; // Standard usage
	cubeDesc.SampleDesc.Count = 1;
	cubeDesc.SampleDesc.Quality = 0;

	// Create the final texture resource to hold the cube map
	Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeMapTexture;
	m_pDevice->CreateTexture2D(&cubeDesc, 0, cubeMapTexture.GetAddressOf());

	// Loop through the individual face textures and copy them,
	// one at a time, to the cube map texure
	for (int i = 0; i < 6; i++)
	{
		// Calculate the subresource position to copy into
		unsigned int subresource = D3D11CalcSubresource(
			0,  // Which mip (zero, since there's only one)
			i,  // Which array element?
			1); // How many mip levels are in the texture?

		// Copy from one resource (texture) to another
		m_pContext->CopySubresourceRegion(
			cubeMapTexture.Get(),  // Destination resource
			subresource,           // Dest subresource index (one of the array elements)
			0, 0, 0,               // XYZ location of copy
			textures[i].Get(),     // Source resource
			0,                     // Source subresource index (we're assuming there's only one)
			0);                    // Source subresource "box" of data to copy (zero means the whole thing)
	}

	// At this point, all of the faces have been copied into the 
	// cube map texture, so we can describe a shader resource view for it
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = cubeDesc.Format;         // Same format as texture
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; // Treat this as a cube!
	srvDesc.TextureCube.MipLevels = 1;        // Only need access to 1 mip
	srvDesc.TextureCube.MostDetailedMip = 0;  // Index of the first mip we want to see

	// Make the SRV
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
	m_pDevice->CreateShaderResourceView(cubeMapTexture.Get(), &srvDesc, cubeSRV.GetAddressOf());

	// Send back the SRV, which is what we need for our shaders
	return cubeSRV;
}