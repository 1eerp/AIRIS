#include "Mesh.hpp"
Mesh::Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::vector<Vertex> vertices, std::vector<uint16_t> indices)
{
	m_vertexStride = sizeof(Vertex);
	m_vertexBufferSize = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());
	m_indexBufferSize = sizeof(uint16_t) * static_cast<uint32_t>(indices.size());
	ThrowIfFailed(D3DCreateBlob(m_vertexBufferSize, &m_vertexBufferCPU));
	CopyMemory(m_vertexBufferCPU->GetBufferPointer(), vertices.data(), m_vertexBufferSize);


	// Indices
	ThrowIfFailed(D3DCreateBlob(m_indexBufferSize, &m_indexBufferCPU));
	CopyMemory(m_indexBufferCPU->GetBufferPointer(), indices.data(), m_indexBufferSize);

	m_vertexBufferGPU = CreateDefaultBuffer(device, cmdList, vertices.data(), m_vertexBufferSize, m_vertexBufferUploader);
	m_indexBufferGPU = CreateDefaultBuffer(device, cmdList, indices.data(), m_indexBufferSize, m_indexBufferUploader);


}

D3D12_VERTEX_BUFFER_VIEW Mesh::VertexBufferView() const
{
	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = m_vertexBufferGPU->GetGPUVirtualAddress();
	vbv.StrideInBytes = m_vertexStride;
	vbv.SizeInBytes = m_vertexBufferSize;

	return vbv;
}

D3D12_INDEX_BUFFER_VIEW Mesh::IndexBufferView() const
{
	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = m_indexBufferGPU->GetGPUVirtualAddress();
	ibv.Format = IndexFormat;
	ibv.SizeInBytes = m_indexBufferSize;

	return ibv;
}

