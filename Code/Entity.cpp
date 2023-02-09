#include "Entity.h"
#include "BufferStructs.h"

using namespace DirectX;

Entity::Entity(std::shared_ptr<Mesh> a_pMesh)
	:m_pMesh(a_pMesh)
{
	m_transform = Transform();
}

std::shared_ptr<Mesh> Entity::GetMesh() { return m_pMesh; }
Transform* Entity::GetTransform() { return &m_transform; }

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> a_pContext, Microsoft::WRL::ComPtr<ID3D11Buffer> a_pVsConstantBuffer)
{
	// Create local data for the constant buffer struct
	VertexShaderExternalData vsData;
	vsData.colorTint = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vsData.world = m_transform.GetWorldMatrix();
	
	// Copy the data by mapping, copying, then unmapping
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	a_pContext->Map(a_pVsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	a_pContext->Unmap(a_pVsConstantBuffer.Get(), 0);

	m_pMesh->Draw(a_pContext);
}
