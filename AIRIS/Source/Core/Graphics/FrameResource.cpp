#include "FrameResource.hpp"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount)
{
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CommandAllocator.GetAddressOf())));

    PassCB = CreateScope<UploadBuffer<PassConstants>>(device, passCount, true);
    ObjectCB = CreateScope<UploadBuffer<ObjectConstants>>(device, objectCount, true);
}
