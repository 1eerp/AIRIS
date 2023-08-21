#include "D3D12API.hpp"
#include <exception>
void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
		throw std::exception();
}