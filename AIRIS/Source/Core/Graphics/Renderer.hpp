#pragma once

#include "Core/Events/IEventListener.hpp"
#include "Core/Window.hpp"
#include "Core/API/RendererAPI.hpp"
#include "Mesh.hpp"
using Microsoft::WRL::ComPtr;



// Will replace and/or refactor API and GRAPHICS code completely, for now it would be nice to see something on the screen
class Renderer : public IEventListener
{
public:
	virtual void Draw() = 0;
	virtual void Update() = 0;

private:
	virtual void Init() = 0;
};


class RRenderer : public Renderer
{
public:
	RRenderer(uint16_t width, uint16_t height);
	~RRenderer();

	virtual void Draw() override;
	virtual void Update() override;

	virtual bool OnWindowResize(IEvent*) override;

private:
	virtual void Init() override;
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateDescriptorHeaps();
	void CreateMesh();
	void CreateInputLayoutAndShaders();
	void CreateRootSignatureAndPSO();
	void ResizeBuffers(unsigned short width, unsigned short height);
	void FlushCommandQueue();
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

private:
	API_CONFIG								m_config;
	API_INFO								m_info;

	ComPtr<IDXGIFactory6>					m_dxgiFactory;
	ComPtr<ID3D12Device>					m_device;
	ComPtr<IDXGISwapChain1>					m_swapChain;

	ComPtr<ID3D12Fence>						m_fence;
	UINT64									m_fenceValue = 0;

	ComPtr<ID3D12CommandQueue>				m_cmdQueue;
	ComPtr<ID3D12CommandAllocator>			m_cmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>		m_cmdList;

	uint8_t									m_curBackBuffer = 0;
	std::vector<ComPtr<ID3D12Resource>>		m_swapChainBuffer = std::vector<ComPtr<ID3D12Resource>>(DEFAULT_SWAPCHAINBUFFERCOUNT);
	ComPtr<ID3D12Resource>					m_depthStencilBuffer;

	ComPtr<ID3D12DescriptorHeap>			m_rtvHeap,
											m_dsvHeap;



	ComPtr<ID3DBlob>						m_vsByteCode = nullptr;
	ComPtr<ID3DBlob>						m_psByteCode = nullptr;

	// ROOT SIGN, INPUT_LAYOUT
	ComPtr<ID3D12RootSignature>				m_rootSignature = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC>	m_inputLayout;
	ComPtr<ID3D12PipelineState>				m_PSO = nullptr;

	// MESH
	std::unique_ptr<Mesh>					m_mesh = nullptr;



	D3D12_VIEWPORT							m_viewport;
	D3D12_RECT								m_scissorRect;


	uint16_t								m_clientWidth,
											m_clientHeight;

};