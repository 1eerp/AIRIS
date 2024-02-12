#pragma once
#include "Core/API/D3D12/Buffer.h"
#include "Core/API/RendererAPI.hpp"
#include "glm/glm.hpp"
#include "ShaderData.hpp"
#include "Util.hpp"

struct FrameResource
{
public:

    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource() = default;

    // Allocator
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;

    // Constant Buffers
    Scope<UploadBuffer<PassConstants>> PassCB = nullptr;
    Scope<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;

    UINT64 Fence = 0;
};