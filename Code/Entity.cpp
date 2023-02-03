#include "Entity.h"
#include "BufferStructs.h"

using namespace DirectX;

Entity::Entity(std::shared_ptr<Mesh> mesh)
	:m_mesh(mesh)
{
	m_transform = Transform();
	m_colorTintValue = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}

std::shared_ptr<Mesh> Entity::GetMesh() { return m_mesh; }

Transform* Entity::GetTransform() { return &m_transform; }

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer)
{
	// Create local data for the constant buffer struct
	VertexShaderExternalData vsData;
	//vsData.offset = m_offsetValue;
	vsData.worldMatrix = m_transform.GetWorldMatrix();
	vsData.colorTint = m_colorTintValue;
	
	// Copy the data by mapping, copying, then unmapping
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	context->Unmap(vsConstantBuffer.Get(), 0);

	// Bind the constant buffer to the right place
	context->VSSetConstantBuffers(
		0, // Which slot (register) to bind the buffer to?
		1, // How many are we activating? Can do multiple at once
		vsConstantBuffer.GetAddressOf()); // Array of buffers (or the address of one)

	m_mesh->Draw();
}
