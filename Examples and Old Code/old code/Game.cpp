
// Old Vertex Struct Definition
struct Vertex
{
	DirectX::XMFLOAT3 Position;	    // The local position of the vertex
	DirectX::XMFLOAT4 Color;        // The color of the vertex
};

void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors=
	const XMFLOAT4 C_BLACK = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	const XMFLOAT4 C_WHITE = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	const XMFLOAT4 C_RED = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	const XMFLOAT4 C_ORANGE = XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f);
	const XMFLOAT4 C_YELLOW = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	const XMFLOAT4 C_CHARTREUSE = XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f);
	const XMFLOAT4 C_GREEN = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	const XMFLOAT4 C_SPRING = XMFLOAT4(0.0f, 1.0f, 0.5f, 1.0f);
	const XMFLOAT4 C_CYAN = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
	const XMFLOAT4 C_AZURE = XMFLOAT4(0.0f, 0.5f, 1.0f, 1.0f);
	const XMFLOAT4 C_BLUE = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	const XMFLOAT4 C_VIOLET = XMFLOAT4(0.5f, 0.0f, 1.0f, 1.0f);
	const XMFLOAT4 C_PINK = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
	const XMFLOAT4 C_MAGENTA = XMFLOAT4(1.0f, 0.0f, 0.5f, 1.0f);

	std::shared_ptr<Material> whiteMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	std::shared_ptr<Material> redMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
	std::shared_ptr<Material> greenMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
	std::shared_ptr<Material> blueMaterial = std::make_shared<Material>(m_pVertexShader, m_pPixelShader, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));

	// Set up the vertices of the triangle we would like to draw
	Vertex triangleVertices[] =
	{
		{ XMFLOAT3(+0.35f, +0.75f, +0.0f), C_RED },
		{ XMFLOAT3(+0.55f, +0.25f, +0.0f), C_GREEN },
		{ XMFLOAT3(+0.25f, +0.25f, +0.0f), C_BLUE },
	};
	// Set up indices, which tell us which vertices to use and in which order
	unsigned int triangleIndices[] = { 0, 1, 2 };
	// Create mesh object
	std::shared_ptr<Mesh> triangleMesh = std::make_shared<Mesh>(triangleVertices, sizeof(triangleVertices) / sizeof(triangleVertices[0]), triangleIndices, sizeof(triangleIndices) / sizeof(triangleIndices[0]), device, context);

	Vertex quadVertices[] =
	{
		{ XMFLOAT3(-0.8f, +0.8f, +0.0f), C_SPRING },
		{ XMFLOAT3(-0.5f, +0.8f, +0.0f), C_BLACK },
		{ XMFLOAT3(-0.5f, +0.5f, +0.0f), C_PINK },
		{ XMFLOAT3(-0.8f, +0.5f, +0.0f), C_WHITE },
	};
	unsigned int quadIndices[] = {
									0, 1, 2,
									0, 2, 3
	};
	std::shared_ptr<Mesh> quadMesh = std::make_shared<Mesh>(quadVertices, sizeof(quadVertices) / sizeof(quadVertices[0]), quadIndices, sizeof(quadIndices) / sizeof(quadIndices[0]), device, context);

	Vertex hexaVerts[] =
	{
		{ XMFLOAT3(+0.0f, +0.0f, +0.0f), C_CHARTREUSE},
		{ XMFLOAT3(+0.15f, -0.15f, +0.0f), C_AZURE},
		{ XMFLOAT3(+0.15f, -0.45f, +0.0f), C_ORANGE},
		{ XMFLOAT3(+0.0f, -0.6f, +0.0f), C_CYAN},
		{ XMFLOAT3(-0.15f, -0.45f, +0.0f), C_YELLOW},
		{ XMFLOAT3(-0.15f, -0.15f, +0.0f), C_VIOLET},
	};
	unsigned int hexaIndices[] = {
									0, 1, 5,
									5, 1, 2,
									5, 2, 4,
									4, 2, 3
	};
	std::shared_ptr<Mesh> hexMesh = std::make_shared<Mesh>(hexaVerts, sizeof(hexaVerts) / sizeof(hexaVerts[0]), hexaIndices, sizeof(hexaIndices) / sizeof(hexaIndices[0]), device, context);

	m_pMeshes.push_back(triangleMesh);
	m_pMeshes.push_back(quadMesh);
	m_pMeshes.push_back(hexMesh);

	// Make a bunch more hexagon entities that share the same mesh and move them
	std::shared_ptr<Entity> extraHex1 = std::make_shared<Entity>(hexMesh, whiteMaterial);
	std::shared_ptr<Entity> extraHex2 = std::make_shared<Entity>(hexMesh, whiteMaterial);
	extraHex1->GetTransform()->Scale(0.25f, 0.25f, 0.0f);
	extraHex1->GetTransform()->MoveAbsolute(0.6f, 0.1f, 0.0f);
	extraHex2->GetTransform()->Scale(0.5f, 0.5f, 0.0f);
	extraHex2->GetTransform()->MoveAbsolute(-0.5f, -0.5f, 0.0f);

	m_pEntities.push_back(std::make_shared<Entity>(triangleMesh, redMaterial));
	m_pEntities.push_back(std::make_shared<Entity>(quadMesh, greenMaterial));
	m_pEntities.push_back(std::make_shared<Entity>(hexMesh, blueMaterial));
	m_pEntities.push_back(extraHex1);
	m_pEntities.push_back(extraHex2);
}

