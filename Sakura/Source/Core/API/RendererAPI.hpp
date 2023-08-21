#pragma once


#define _D3DAPI 1
#if _D3DAPI

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#include "D3D12/D3D12API.hpp"
#endif