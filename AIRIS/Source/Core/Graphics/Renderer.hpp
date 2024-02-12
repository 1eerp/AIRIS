#pragma once

#include "Core/Events/IEventListener.hpp"
#include "Core/Window.hpp"
#include "Core/API/RendererAPI.hpp"
#include "FrameResource.hpp"
#include "Mesh.hpp"
#include "RenderItem.hpp"
#include "CameraController.hpp"
#include "RTHelper.hpp"
#include "ShaderData.hpp"
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

struct RTConstants
{
	// Booleans are 32BIT(4 Bytes) in HLSL 
	uint32_t	AccumlateSamples	= false, // bool
				ResetOutput			= false, // bool
				AccumulatedSamples	= 0,
				MaxRayBounces		= 7,
				RandSeed			= 0;
};

class RRenderer : public Renderer
{
public:
	RRenderer(uint16_t width, uint16_t height);
	~RRenderer();

	virtual void Update() override;
	virtual void Draw() override;

	virtual bool OnWindowResize(IEvent*) override;
	virtual bool OnKeyDown(IEvent* e) override;
	virtual bool OnMouseDown(IEvent* e) override;
	virtual bool OnMouseWheel(IEvent* e) override;


private:
	virtual void Init() override;
	void CreateFrameResources();
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateDescriptorHeaps();
	void CreateCBVDescriptorHeap();
	void CreateObjects();
	void CreateRenderItems();
	void CreateShader();
	void CreateRootSignatureAndPSOs();
	void CreateConstantBufferViews();
	void ResizeBuffers(uint16_t width, uint16_t height);

	void DrawRenderItems();

	void UpdatePassConstants();
	void UpdateConstantBuffers();
	void FlushCommandQueue();
	void FlushCurrentFRCommands();

	ID3D12Resource* CurrentBackBuffer() const;
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

	// Command Interfaces
	ComPtr<ID3D12CommandQueue>				m_cmdQueue;
	ComPtr<ID3D12CommandAllocator>			m_cmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>		m_cmdList;

	// Resources and Descriptors
	uint8_t									m_curBackBuffer = 0;
	std::vector<ComPtr<ID3D12Resource>>		m_swapChainBuffers = std::vector<ComPtr<ID3D12Resource>>(DEFAULT_SWAPCHAINBUFFERCOUNT);
	ComPtr<ID3D12Resource>					m_depthStencilBuffer;

	// Constant Buffers
	Ref<UploadBuffer<PassConstants>>		m_passConstantBuffer;

	static const int						s_frameResourcesCount = 3;
	std::vector<Scope<FrameResource>>		m_frameResources;
	FrameResource*							m_curFrameResource = nullptr;
	int										m_curFrameResourceIndex = 0;

	// Descriptor Heaps
	ComPtr<ID3D12DescriptorHeap>			m_rtvHeap,
											m_dsvHeap,
											m_cbvHeap; // CBV
	// Shader Bytecode
	ComPtr<ID3DBlob>						m_vsByteCode = nullptr,
											m_psByteCode = nullptr;

	// ROOT SIGNATURE, INPUT_LAYOUT
	ComPtr<ID3D12RootSignature>				m_opaqueRootSig = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC>	m_inputLayout;
	ComPtr<ID3D12PipelineState>				m_PSO = nullptr;

	D3D12_VIEWPORT							m_viewport;
	D3D12_RECT								m_scissorRect;
	unsigned short							m_clientWidth,
											m_clientHeight;

	// Constant Buffer Data
	PassConstants							m_mainPassConstants;

	// Meshes
	std::unique_ptr<Mesh>					m_mesh = nullptr;

	// List of all the render items.
	std::vector<Scope<RenderItem>>			m_renderItems;

	// Render items divided by PSO.
	std::vector<RenderItem*>				m_opaqueRItems;
	std::vector<RenderItem*>				m_transparentRItems;

	glm::vec3								m_meshPos = {0.f, 0.f, 0.f};

	// EDITOR CAMERA CONTROLLER
	EditorCC								m_controller;
};