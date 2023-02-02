#include "Mesh.h"

Mesh::Mesh(Vertex* vertexArray, int numVertices, unsigned int* indexArray, int numIndices, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	this->m_indexBufferCount = numIndices;
	this->m_context = context;

	// Create a VERTEX BUFFER
	// - This holds the vertex data of triangles for a single object
	// - This buffer is created on the GPU, which is where the data needs to
	//    be if we want the GPU to act on it (as in: draw it to the screen)
		// First, we need to describe the buffer we want Direct3D to make on the GPU
		//  - Note that this variable is created on the stack since we only need it once
		//  - After the buffer is created, this description variable is unnecessary
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
	vbd.ByteWidth = sizeof(Vertex) * numVertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells Direct3D this is a vertex buffer
	vbd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	// Create the proper struct to hold the initial vertex data
	// - This is how we initially fill the buffer with data
	// - Essentially, we're specifying a pointer to the data to copy
	D3D11_SUBRESOURCE_DATA initialVertexData = {};
	initialVertexData.pSysMem = vertexArray; // pSysMem = Pointer to System Memory
	// Actually create the buffer on the GPU with the initial data
	// - Once we do this, we'll NEVER CHANGE DATA IN THE BUFFER AGAIN
	device->CreateBuffer(&vbd, &initialVertexData, m_vertexBuffer.GetAddressOf());
	
	// Create an INDEX BUFFER
	// - This holds indices to elements in the vertex buffer
	// - This is most useful when vertices are shared among neighboring triangles
	// - This buffer is created on the GPU, which is where the data needs to
	//    be if we want the GPU to act on it (as in: draw it to the screen)
		// Describe the buffer, as we did above, with two major differences
		//  - Byte Width (3 unsigned integers vs. 3 whole vertices)
		//  - Bind Flag (used as an index buffer instead of a vertex buffer) 
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
	ibd.ByteWidth = sizeof(unsigned int) * numIndices;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;	// Tells Direct3D this is an index buffer
	ibd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	// Specify the initial data for this buffer, similar to above
	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = indexArray; // pSysMem = Pointer to System Memory
	// Actually create the buffer with the initial data
	// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	device->CreateBuffer(&ibd, &initialIndexData, m_indexBuffer.GetAddressOf());
}

Mesh::~Mesh() {}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer() { return m_vertexBuffer; }
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer() { return m_indexBuffer; }
int Mesh::GetIndexCount() { return m_indexBufferCount; }

void Mesh::Draw()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// Set buffers in the input assembler (IA) stage
	this->m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	this->m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Tell Direct3D to draw
	this->m_context->DrawIndexed(
		this->m_indexBufferCount,     // The number of indices to use (we could draw a subset if we wanted)
		0,     // Offset to the first index we want to use
		0);    // Offset to add to each index when looking up vertices
}
