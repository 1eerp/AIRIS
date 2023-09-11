#pragma once

#include "Util.hpp"
#include "Core/API/RendererAPI.hpp"
#include "Vertex.hpp"
static UINT CalcConstantBufferByteSize(UINT byteSize)
{
    // Constant buffers must be a multiple of the minimum hardware
    // allocation size (usually 256 bytes).  So round up to nearest
    // multiple of 256.  We do this by adding 255 and then masking off
    // the lower 2 bytes which store all bits < 256.
    // Example: Suppose byteSize = 300.
    // (300 + 255) & ~255
    // 555 & ~255
    // 0x022B & ~0x00ff
    // 0x022B & 0xff00
    // 0x0200
    // 512
    return (byteSize + 255) & ~255;
}

struct SubMesh
{
	uint32_t IndexCount = 0;
	uint32_t StartIndexLocation = 0;
	uint32_t BaseVertexLocation = 0;
};

using Microsoft::WRL::ComPtr;
class Mesh
{
public:
	Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::vector<Vertex> vertices, std::vector<uint16_t> indices);

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const;

public:
	std::string m_name;
	std::unordered_map<std::string, SubMesh> m_subMeshes;

private:
	// Give it a name so we can look it up by name.

	// System memory copies.  Use Blobs because the vertex/index format can be generic.
	// It is up to the client to cast appropriately.  
	ComPtr<ID3DBlob> m_vertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> m_indexBufferCPU = nullptr;
	ComPtr<ID3D12Resource> m_vertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> m_indexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> m_vertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> m_indexBufferUploader = nullptr;

	// Data about the buffers (IN BYTES)
	UINT m_vertexStride = 0;
	UINT m_vertexBufferSize = 0;
	UINT m_indexBufferSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;

	// A MeshGeometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.

	// We can free this memory after we finish upload to the GPU.
	void DisposeUploaders()
	{
		m_vertexBufferUploader = nullptr;
		m_indexBufferUploader = nullptr;
	}
};


