#include <vector>
#include <fstream>

#include "Mesh.h"

using namespace DirectX;

Mesh::Mesh(Vertex* a_vertexArray, int a_vertexCount, unsigned int* a_indexArray, int a_indexCount, Microsoft::WRL::ComPtr<ID3D11Device> a_pDevice)
{
	CreateBuffers(a_vertexArray, a_vertexCount, a_indexArray, a_indexCount, a_pDevice);
}

Mesh::Mesh(const std::wstring a_filename, Microsoft::WRL::ComPtr<ID3D11Device> a_pDevice)
{
	// The following code was written by Chris Cascioli:
	// File input object
	std::ifstream obj(a_filename);

	// Check for successful open
	if (!obj.is_open()) {
		printf("Error in opening file");
		return;
	}

	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;	// Positions from the file
	std::vector<XMFLOAT3> normals;		// Normals from the file
	std::vector<XMFLOAT2> uvs;		// UVs from the file
	std::vector<Vertex> verts;		// Verts we're assembling
	std::vector<UINT> indices;		// Indices of these verts
	int vertCounter = 0;			// Count of vertices
	int indexCounter = 0;			// Count of indices
	char chars[100];			// String for line reading

	// Still have data left?
	while (obj.good())
	{
		// Get the line (100 characters should be more than enough)
		obj.getline(chars, 100);

		// Check the type of line
		if (chars[0] == 'v' && chars[1] == 'n')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 norm;
			sscanf_s(
				chars,
				"vn %f %f %f",
				&norm.x, &norm.y, &norm.z);

			// Add to the list of normals
			normals.push_back(norm);
		}
		else if (chars[0] == 'v' && chars[1] == 't')
		{
			// Read the 2 numbers directly into an XMFLOAT2
			XMFLOAT2 uv;
			sscanf_s(
				chars,
				"vt %f %f",
				&uv.x, &uv.y);

			// Add to the list of uv's
			uvs.push_back(uv);
		}
		else if (chars[0] == 'v')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 pos;
			sscanf_s(
				chars,
				"v %f %f %f",
				&pos.x, &pos.y, &pos.z);

			// Add to the positions
			positions.push_back(pos);
		}
		else if (chars[0] == 'f')
		{
			// Read the face indices into an array
			// NOTE: This assumes the given obj file contains
			//  vertex positions, uv coordinates AND normals.
			unsigned int i[12];
			int numbersRead = sscanf_s(
				chars,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&i[0], &i[1], &i[2],
				&i[3], &i[4], &i[5],
				&i[6], &i[7], &i[8],
				&i[9], &i[10], &i[11]);

			// If we only got the first number, chances are the OBJ
			// file has no UV coordinates.  This isn't great, but we
			// still want to load the model without crashing, so we
			// need to re-read a different pattern (in which we assume
			// there are no UVs denoted for any of the vertices)
			if (numbersRead == 1)
			{
				// Re-read with a different pattern
				numbersRead = sscanf_s(
					chars,
					"f %d//%d %d//%d %d//%d %d//%d",
					&i[0], &i[2],
					&i[3], &i[5],
					&i[6], &i[8],
					&i[9], &i[11]);

				// The following indices are where the UVs should 
				// have been, so give them a valid value
				i[1] = 1;
				i[4] = 1;
				i[7] = 1;
				i[10] = 1;

				// If we have no UVs, create a single UV coordinate
				// that will be used for all vertices
				if (uvs.size() == 0)
					uvs.push_back(XMFLOAT2(0, 0));
			}

			// - Create the verts by looking up
			//    corresponding data from vectors
			// - OBJ File indices are 1-based, so
			//    they need to be adusted
			Vertex v1;
			v1.Position = positions[i[0] - 1];
			v1.UV = uvs[i[1] - 1];
			v1.Normal = normals[i[2] - 1];

			Vertex v2;
			v2.Position = positions[i[3] - 1];
			v2.UV = uvs[i[4] - 1];
			v2.Normal = normals[i[5] - 1];

			Vertex v3;
			v3.Position = positions[i[6] - 1];
			v3.UV = uvs[i[7] - 1];
			v3.Normal = normals[i[8] - 1];

			// The model is most likely in a right-handed space,
			// especially if it came from Maya.  We want to convert
			// to a left-handed space for DirectX.  This means we 
			// need to:
			//  - Invert the Z position
			//  - Invert the normal's Z
			//  - Flip the winding order
			// We also need to flip the UV coordinate since DirectX
			// defines (0,0) as the top left of the texture, and many
			// 3D modeling packages use the bottom left as (0,0)

			// Flip the UV's since they're probably "upside down"
			v1.UV.y = 1.0f - v1.UV.y;
			v2.UV.y = 1.0f - v2.UV.y;
			v3.UV.y = 1.0f - v3.UV.y;

			// Flip Z (LH vs. RH)
			v1.Position.z *= -1.0f;
			v2.Position.z *= -1.0f;
			v3.Position.z *= -1.0f;

			// Flip normal's Z
			v1.Normal.z *= -1.0f;
			v2.Normal.z *= -1.0f;
			v3.Normal.z *= -1.0f;

			// Add the verts to the vector (flipping the winding order)
			verts.push_back(v1);
			verts.push_back(v3);
			verts.push_back(v2);
			vertCounter += 3;

			// Add three more indices
			indices.push_back(indexCounter); indexCounter += 1;
			indices.push_back(indexCounter); indexCounter += 1;
			indices.push_back(indexCounter); indexCounter += 1;

			// Was there a 4th face?
			// - 12 numbers read means 4 faces WITH uv's
			// - 8 numbers read means 4 faces WITHOUT uv's
			if (numbersRead == 12 || numbersRead == 8)
			{
				// Make the last vertex
				Vertex v4;
				v4.Position = positions[i[9] - 1];
				v4.UV = uvs[i[10] - 1];
				v4.Normal = normals[i[11] - 1];

				// Flip the UV, Z pos and normal's Z
				v4.UV.y = 1.0f - v4.UV.y;
				v4.Position.z *= -1.0f;
				v4.Normal.z *= -1.0f;

				// Add a whole triangle (flipping the winding order)
				verts.push_back(v1);
				verts.push_back(v4);
				verts.push_back(v3);
				vertCounter += 3;

				// Add three more indices
				indices.push_back(indexCounter); indexCounter += 1;
				indices.push_back(indexCounter); indexCounter += 1;
				indices.push_back(indexCounter); indexCounter += 1;
			}
		}
	}

	// Close the file and create the actual buffers
	obj.close();


	CreateBuffers(&verts[0], vertCounter, &indices[0], vertCounter, a_pDevice);
}

Mesh::~Mesh() {}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer() { return m_pVertexBuffer; }
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer() { return m_pIndexBuffer; }
int Mesh::GetIndexCount() { return m_indexBufferCount; }

void Mesh::CreateBuffers(Vertex* a_vertexArray, int a_vertexCount, unsigned int* a_indexArray, int a_indexCount, Microsoft::WRL::ComPtr<ID3D11Device> a_pDevice)
{
	this->CalculateTangents(a_vertexArray, a_vertexCount, a_indexArray, a_indexCount);
	this->m_indexBufferCount = a_indexCount;

	// Create a VERTEX BUFFER
	// - This holds the vertex data of triangles for a single object
	// - This buffer is created on the GPU, which is where the data needs to
	//    be if we want the GPU to act on it (as in: draw it to the screen)
		// First, we need to describe the buffer we want Direct3D to make on the GPU
		//  - Note that this variable is created on the stack since we only need it once
		//  - After the buffer is created, this description variable is unnecessary
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
	vbd.ByteWidth = sizeof(Vertex) * a_vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells Direct3D this is a vertex buffer
	vbd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	// Create the proper struct to hold the initial vertex data
	// - This is how we initially fill the buffer with data
	// - Essentially, we're specifying a pointer to the data to copy
	D3D11_SUBRESOURCE_DATA initialVertexData = {};
	initialVertexData.pSysMem = a_vertexArray; // pSysMem = Pointer to System Memory
	// Actually create the buffer on the GPU with the initial data
	// - Once we do this, we'll NEVER CHANGE DATA IN THE BUFFER AGAIN
	a_pDevice->CreateBuffer(&vbd, &initialVertexData, m_pVertexBuffer.GetAddressOf());

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
	ibd.ByteWidth = sizeof(unsigned int) * a_indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;	// Tells Direct3D this is an index buffer
	ibd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	// Specify the initial data for this buffer, similar to above
	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = a_indexArray; // pSysMem = Pointer to System Memory
	// Actually create the buffer with the initial data
	// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	a_pDevice->CreateBuffer(&ibd, &initialIndexData, m_pIndexBuffer.GetAddressOf());
}

void Mesh::CalculateTangents(Vertex* a_verts, int a_numVerts, unsigned int* a_indices, int a_numIndices)
{// Reset tangents
	for (int i = 0; i < a_numVerts; i++)
	{
		a_verts[i].Tangent = XMFLOAT3(0, 0, 0);
	}

	// Calculate tangents one whole triangle at a time
	for (int i = 0; i < a_numIndices;)
	{
		// Grab indices and vertices of first triangle
		unsigned int i1 = a_indices[i++];
		unsigned int i2 = a_indices[i++];
		unsigned int i3 = a_indices[i++];
		Vertex* v1 = &a_verts[i1];
		Vertex* v2 = &a_verts[i2];
		Vertex* v3 = &a_verts[i3];

		// Calculate vectors relative to triangle positions
		float x1 = v2->Position.x - v1->Position.x;
		float y1 = v2->Position.y - v1->Position.y;
		float z1 = v2->Position.z - v1->Position.z;

		float x2 = v3->Position.x - v1->Position.x;
		float y2 = v3->Position.y - v1->Position.y;
		float z2 = v3->Position.z - v1->Position.z;

		// Do the same for vectors relative to triangle uv's
		float s1 = v2->UV.x - v1->UV.x;
		float t1 = v2->UV.y - v1->UV.y;

		float s2 = v3->UV.x - v1->UV.x;
		float t2 = v3->UV.y - v1->UV.y;

		// Create vectors for tangent calculation
		float r = 1.0f / (s1 * t2 - s2 * t1);

		float tx = (t2 * x1 - t1 * x2) * r;
		float ty = (t2 * y1 - t1 * y2) * r;
		float tz = (t2 * z1 - t1 * z2) * r;

		// Adjust tangents of each vert of the triangle
		v1->Tangent.x += tx;
		v1->Tangent.y += ty;
		v1->Tangent.z += tz;

		v2->Tangent.x += tx;
		v2->Tangent.y += ty;
		v2->Tangent.z += tz;

		v3->Tangent.x += tx;
		v3->Tangent.y += ty;
		v3->Tangent.z += tz;
	}

	// Ensure all of the tangents are orthogonal to the normals
	for (int i = 0; i < a_numVerts; i++)
	{
		// Grab the two vectors
		XMVECTOR normal = XMLoadFloat3(&a_verts[i].Normal);
		XMVECTOR tangent = XMLoadFloat3(&a_verts[i].Tangent);

		// Use Gram-Schmidt orthonormalize to ensure
		// the normal and tangent are exactly 90 degrees apart
		tangent = XMVector3Normalize(tangent - normal * XMVector3Dot(normal, tangent));

		// Store the tangent
		XMStoreFloat3(&a_verts[i].Tangent, tangent);
	}
}

void Mesh::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> a_pContext)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// Set buffers in the input assembler (IA) stage
	a_pContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	a_pContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Tell Direct3D to draw
	a_pContext->DrawIndexed(
		this->m_indexBufferCount,     // The number of indices to use (we could draw a subset if we wanted)
		0,     // Offset to the first index we want to use
		0);    // Offset to add to each index when looking up vertices
}