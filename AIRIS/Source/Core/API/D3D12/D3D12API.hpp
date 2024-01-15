#pragma once

#include <wrl/client.h>			// Contains ComPtr
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <d3d12.h>


// From official microsoft helper repo
#include "d3dx12.h"

void ThrowIfFailed(HRESULT hr);
#define DEFAULT_SWAPCHAINBUFFERCOUNT 2

struct DESCRIPTOR_SIZES
{
	UINT		RTV,
				DSV;
	union
	{
		UINT	CBV,
				SRV,
				UAV;
	};
};

struct API_CONFIG
{
	D3D_FEATURE_LEVEL FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_0;
	DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	UINT SwapChainBufferCount = DEFAULT_SWAPCHAINBUFFERCOUNT;

	BOOL	MSAAState = false;
	UINT	MSAASampleCount = 4;
	UINT	MSAAQuality = 0;

};

struct API_INFO
{
	DESCRIPTOR_SIZES DescSizes;
	D3D_FEATURE_LEVEL MaxFeatureLevel;
};
