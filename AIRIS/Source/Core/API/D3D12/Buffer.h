#pragma once
#include "D3D12API.hpp"
#include "D3DUtil.hpp"

using Microsoft::WRL::ComPtr;
ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);


template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) :
		m_isCB(isConstantBuffer)
	{
		m_elementByteSize = sizeof(T);

		if (isConstantBuffer)
			m_elementByteSize = CalcConstantBufferByteSize(sizeof(T));

		CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_UPLOAD);
		D3D12_RESOURCE_DESC bufDesc = CD3DX12_RESOURCE_DESC::Buffer(UINT64(m_elementByteSize) * elementCount);
		ThrowIfFailed(device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &bufDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_uploadBuffer)));

		ThrowIfFailed(m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));

	}

	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
	~UploadBuffer()
	{
		if (m_uploadBuffer != nullptr)
			m_uploadBuffer->Unmap(0, nullptr);

		m_mappedData = nullptr;
	}

	ID3D12Resource* Resource()const
	{
		return m_uploadBuffer.Get();
	}

	void CopyData(int elementIndex, const T& data)
	{
		memcpy(&m_mappedData[elementIndex * m_elementByteSize], &data, sizeof(T));
	}

private:
	ComPtr<ID3D12Resource> m_uploadBuffer;
	BYTE* m_mappedData = nullptr;

	UINT m_elementByteSize = 0;
	bool m_isCB = false;
};