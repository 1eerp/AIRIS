#include "Mesh.hpp"
ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);
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

ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
	ComPtr<ID3D12Resource> defaultBuffer;

	D3D12_HEAP_PROPERTIES defaultProps = {};
	defaultProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	defaultProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	defaultProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	defaultProps.VisibleNodeMask = 1;
	defaultProps.CreationNodeMask = 1;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Alignment = 0;
	bufferDesc.Width = byteSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// Create the actual default buffer resource.
	ThrowIfFailed(device->CreateCommittedResource(
		&defaultProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())));


	D3D12_HEAP_PROPERTIES uploadProps = {};
	uploadProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	uploadProps.VisibleNodeMask = 1;
	uploadProps.CreationNodeMask = 1;
	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 

	ThrowIfFailed(device->CreateCommittedResource(
		&uploadProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())));


	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;


	D3D12_RESOURCE_BARRIER barrier1{};
	barrier1.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier1.Transition.pResource = defaultBuffer.Get();
	barrier1.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	barrier1.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier1.Transition.Subresource = 0xffffffff;
	barrier1.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	D3D12_RESOURCE_BARRIER barrier2{};
	barrier2.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier2.Transition.pResource = defaultBuffer.Get();
	barrier2.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier2.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier2.Transition.Subresource = 0xffffffff;
	barrier2.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	cmdList->ResourceBarrier(1, &barrier1);
	UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &barrier2);



	return defaultBuffer;
}