#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>

#include "Vertex.h"

class Mesh {
public:
	Mesh(Vertex* a_vertexArray, int a_vertexCount, unsigned int* a_indexArray, int a_indexCount, Microsoft::WRL::ComPtr<ID3D11Device> a_pDevice);
	Mesh(const std::wstring a_filename, Microsoft::WRL::ComPtr<ID3D11Device> a_pDevice);
	~Mesh();

	/* Returns the pointer to the vertex buffer object */
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();

	/* Returns the pointer to the index buffer object */
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();

	/* Returns the number of indices this mesh contains */
	int GetIndexCount();

	/* Sets the buffers and tells DirectX to draw the correct number of indices */
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> a_pContext);

private:
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
	int m_indexBufferCount;

	void CreateBuffers(Vertex* a_vertexArray, int a_vertexCount, unsigned int* a_indexArray, int a_indexCount, Microsoft::WRL::ComPtr<ID3D11Device> a_pDevice);
	void CalculateTangents(Vertex* a_verts, int a_numVerts, unsigned int* a_indices, int a_numIndices);
};