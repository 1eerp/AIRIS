#pragma once
#include "Core/API/RendererAPI.hpp"
#include "glm/glm.hpp"

class Mesh;

struct RenderItem
{
	// Item Position, Rotation and Scale
	glm::mat4 ModelMatrix = glm::mat4(1.f);
	// Item Mesh
	Mesh* Mesh = nullptr;
	// Item Primitive Type
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	// Number of Frame Resources that hold incorrect data, and require update
	int DirtyFRCount = 3;
	// Constant Buffer location offset
	unsigned int ConstantBufferIndex = 0xffffffff;
	// Primitive vertex data count and locations
	unsigned int IndexCount = 0;
	unsigned int StartIndexLocation = 0;
	unsigned int BaseVertexLocation = 0;
};