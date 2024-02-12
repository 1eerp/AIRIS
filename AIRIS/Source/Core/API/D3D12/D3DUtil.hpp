#pragma once

#include <string>
#include "D3D12API.hpp"

UINT CalcConstantBufferByteSize(UINT byteSize);
Microsoft::WRL::ComPtr<ID3DBlob> D12UTILCompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target);