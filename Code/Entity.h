#pragma once

#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>

#include "Mesh.h"
#include "Transform.h"

class Entity
{
public:
	Entity(std::shared_ptr<Mesh> mesh);


private:
	std::shared_ptr<Mesh> m_mesh;
};

