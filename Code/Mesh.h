#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"

class Mesh {
public:
	// Constructor
	Mesh(Vertex* vertexArray, int numVerts, unsigned int* indexArray, int numIndices, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	// Destructor
	~Mesh();

	/* USAGE: returns the pointer to the vertex buffer object */
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();

	/* USAGE: returns the pointer to the index buffer object */
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();

	/* USAGE: returns the number of indices this mesh contains */
	int GetIndexCount();

	/* USAGE: sets the buffers and tells DirectX to draw the correct number of indices */
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
	int m_indexBufferCount;

	/* USAGE: ---
	ARGUMENTS: ---
	OUTPUT: ---
	*/
	//void MethodName();
};