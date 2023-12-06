#include "Renderer.hpp"
#include "Core/Log.hpp"
#include "Core/Events/EventSystem.hpp"
#include "Core/Events/ApplicationEvents.hpp"
#include "Core/Events/InputEvents.h"
#include "Core/Input/Input.hpp"
#include <random>


ComPtr<ID3DBlob> SCompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target);

RRenderer::RRenderer(uint16_t width, uint16_t height)
	:m_clientWidth(width), m_clientHeight(height)
{
	try { 
		Init();
	}
	catch(std::exception e)
	{
		CORE_ERROR(e.what());
		DebugBreak();
	}
	EVENTSYSTEM->RegisterEventListener(EventType::WindowResize, dynamic_cast<IEventListener*>(this), reinterpret_cast<EventSystem::EventCallback>(&RRenderer::OnWindowResize));
	EVENTSYSTEM->RegisterEventListener(EventType::KeyPressed, dynamic_cast<IEventListener*>(this), reinterpret_cast<EventSystem::EventCallback>(&RRenderer::OnKeyDown));
	EVENTSYSTEM->RegisterEventListener(EventType::MouseScrolled, dynamic_cast<IEventListener*>(this), reinterpret_cast<EventSystem::EventCallback>(&RRenderer::OnMouseWheel));
	EVENTSYSTEM->RegisterEventListener(EventType::MouseButtonPressed, dynamic_cast<IEventListener*>(this), reinterpret_cast<EventSystem::EventCallback>(&RRenderer::OnMouseDown));


	CORE_INFO("Renderer Intialized");
}

RRenderer::~RRenderer()
{
	if (m_device.Get() != nullptr)
		FlushCommandQueue();
}

void RRenderer::Update()
{

	Draw();
}

void RRenderer::Draw()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

	ThrowIfFailed(m_cmdAlloc->Reset());
	ThrowIfFailed(m_cmdList->Reset(m_cmdAlloc.Get(), m_PSO.Get()));

	m_cmdList->RSSetViewports(1, &m_viewport);
	m_cmdList->RSSetScissorRects(1, &m_scissorRect);
	D3D12_RESOURCE_BARRIER b = CD3DX12_RESOURCE_BARRIER::Transition(m_swapChainBuffers[m_curBackBuffer].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_cmdList->ResourceBarrier(1, &b);
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_cmdList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	m_cmdList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	m_cmdList->OMSetRenderTargets(1, &rtv, true, &dsv);
	m_cmdList->SetGraphicsRootSignature(m_opaqueRootSig.Get());

	D3D12_VERTEX_BUFFER_VIEW views[1] = { m_mesh->VertexBufferView() };
	m_cmdList->IASetVertexBuffers(0, 1, views);
	auto ibv = m_mesh->IndexBufferView();
	m_cmdList->IASetIndexBuffer(&ibv);
	m_cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto& m = m_mesh->m_subMeshes["Triangle"];
	m_cmdList->DrawIndexedInstanced(m.IndexCount, 1, m.StartIndexLocation, m.BaseVertexLocation, 0);
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_swapChainBuffers[m_curBackBuffer].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_cmdList->ResourceBarrier(1, &barrier);

	// Done recording commands.
	ThrowIfFailed(m_cmdList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_cmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(m_swapChain->Present(0, 0));
	m_curBackBuffer = (m_curBackBuffer + 1) % m_config.SwapChainBufferCount;

	FlushCommandQueue();
}

bool RRenderer::OnWindowResize(IEvent* e)
{
	WindowResizeEvent* event = dynamic_cast<WindowResizeEvent*>(e);
	uint16_t clientWidth = event->GetWidth(), clientHeight = event->GetHeight();
	//if (clientWidth == m_clientWidth && clientHeight == m_clientHeight)
	//	return false;
	m_clientWidth = clientWidth, m_clientHeight = clientHeight;
	unsigned int xGroups = static_cast<unsigned int>(ceilf(m_clientWidth / 256.0f));
	CORE_INFO("Resizing Buffers, Width: {}, Height: {}, X Dispatch Groups(256 warp size):{}", m_clientWidth, m_clientWidth, xGroups);
	ResizeBuffers(m_clientWidth, m_clientHeight);

	return false;
}

bool RRenderer::OnKeyDown(IEvent* e)
{
	KeyPressedEvent* event = dynamic_cast<KeyPressedEvent*>(e);
	switch (event->GetKeyCode())
	{
		default:
			break;
	}
	
	return false;
}

bool RRenderer::OnMouseWheel(IEvent* e)
{
	MouseScrolledEvent* scrollEvent = dynamic_cast<MouseScrolledEvent*>(e);
	float delta = scrollEvent->GetScrollDelta() / 120.f; 
	
	return false;
}


bool RRenderer::OnMouseDown(IEvent*e)
{
	MouseButtonPressedEvent* mouseEvent = dynamic_cast<MouseButtonPressedEvent*>(e);
	
	std::array<std::string, 3> mwheelModes = {"FOV", "FocalDist", "DefocusAngle"};
	auto button = mouseEvent->GetMouseButton();
	
	return false;
}

void RRenderer::Init()
{
	UINT factoryFlags = 0;
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug1> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			debugController->SetEnableSynchronizedCommandQueueValidation(true);
		}
	}
#endif

	// Create IDXGIFactory
	ThrowIfFailed(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

	ComPtr<IDXGIAdapter3> adapter;
	m_dxgiFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_1,IID_PPV_ARGS(&m_device));

	if (FAILED(hardwareResult))
		DebugBreak();

	// Fence
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

	// Descriptor Sizes
	m_info.DescSizes.RTV = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_info.DescSizes.DSV = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_info.DescSizes.CBV = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CreateCommandObjects();
	CreateSwapChain();
	CreateDescriptorHeaps();
	ResizeBuffers(m_clientWidth, m_clientHeight);
	CreateConstantBuffers();
	
	// VertexBuffer InputLayout
	m_inputLayout =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},

	};

	m_cmdList->Reset(m_cmdAlloc.Get(), nullptr);
	CreateObjects();
	CreateShader();
	CreateRootSignatureAndPSOs();
	m_cmdList->Close();

	ID3D12CommandList* cmdLists[] = { m_cmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(1, cmdLists);

	// Wait for Intialization
	FlushCommandQueue();

}


void RRenderer::CreateCommandObjects()
{
	// Command Queue
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_cmdQueue)));

	// Command allocator and list
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_cmdAlloc.GetAddressOf())));
	ThrowIfFailed(m_device->CreateCommandList(NULL, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc.Get(), nullptr, IID_PPV_ARGS(m_cmdList.GetAddressOf())));
	m_cmdList->Close();
}

void RRenderer::CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC1 desc1{};
	desc1.Width = m_clientWidth;
	desc1.Height = m_clientHeight;
	desc1.Format = m_config.BackBufferFormat;
	desc1.SampleDesc = {1, 0};
	desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc1.BufferCount = m_config.SwapChainBufferCount;
	desc1.Scaling = DXGI_SCALING_NONE;
	desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	m_swapChain.Reset();
	ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(m_cmdQueue.Get(), SWindow::GetInstance()->GetWindowHandle(), &desc1, nullptr, nullptr,m_swapChain.GetAddressOf()));
}

void RRenderer::CreateDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {}, dsvDesc{}, srvDesc{};
	rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDesc.NumDescriptors = m_config.SwapChainBufferCount;
	
	dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDesc.NumDescriptors = 1;
	
	srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvDesc.NumDescriptors = 1;

	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&m_rtvHeap)));
	ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvDesc, IID_PPV_ARGS(&m_dsvHeap)));
	ThrowIfFailed(m_device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&m_srvHeap)));
}

void RRenderer::CreateShader()
{
	m_vsByteCode = SCompileShader(L"..\\..\\Assets\\Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	m_psByteCode = SCompileShader(L"..\\..\\Assets\\Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");
}

void RRenderer::CreateObjects()
{
	auto aspectRatio = SWindow::GetInstance()->GetAspectRatio();
	std::vector<Vertex> vertices(3);
	vertices =
	{
		Vertex({ glm::vec3(0.0f, 0.25f * aspectRatio, 0.0f), glm::vec4(1.f, 0.f, 0.f, 1.f)}),
		Vertex({ glm::vec3(0.25f, -0.25f * aspectRatio, 0.0f), glm::vec4(0.f, 1.f, 0.f, 1.f)}),
		Vertex({ glm::vec3(-0.25f, -0.25f * aspectRatio, 0.0f), glm::vec4(0.f, 0.f, 1.f, 1.f)}),
	};

	std::vector<std::uint16_t> indices =
	{
		0,1,2
	};
	m_mesh = std::make_unique<Mesh>(m_device.Get(), m_cmdList.Get(), vertices, indices);
	m_mesh->m_name = "TriangleMesh";

	SubMesh sm;
	sm.BaseVertexLocation = 0;
	sm.StartIndexLocation = 0;
	sm.IndexCount = static_cast<uint32_t>(indices.size());
	m_mesh->m_subMeshes["Triangle"] = sm;
}

void RRenderer::CreateRootSignatureAndPSOs()
{
	// ROOT SIGNATURE
	std::array<CD3DX12_DESCRIPTOR_RANGE, 1> ranges;
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_ROOT_PARAMETER params[1];
	params[0].InitAsDescriptorTable(static_cast<UINT>(ranges.size()), &ranges[0]);
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, params, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSerializerBlob = nullptr,
					 errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, rootSerializerBlob.GetAddressOf(), errorBlob.GetAddressOf());
	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	ThrowIfFailed(hr);
	ThrowIfFailed(m_device->CreateRootSignature(NULL, rootSerializerBlob->GetBufferPointer(), rootSerializerBlob->GetBufferSize(), IID_PPV_ARGS(&m_opaqueRootSig)));
	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { };
	psoDesc.InputLayout = { m_inputLayout.data(), (uint32_t)m_inputLayout.size() };
	psoDesc.pRootSignature = m_opaqueRootSig.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
		m_vsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
		m_psByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_config.BackBufferFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = m_config.DepthStencilFormat;
	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSO)));
	m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSO));

}

void RRenderer::CreateConstantBuffers()
{

}

void RRenderer::ResizeBuffers(uint16_t width, uint16_t height)
{
	assert(m_device);
	assert(m_swapChain);
	assert(m_cmdList);

	// Flush before changing any resources.
	FlushCommandQueue();

	ThrowIfFailed(m_cmdList->Reset(m_cmdAlloc.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (unsigned int i = 0; i < m_config.SwapChainBufferCount; ++i)
		m_swapChainBuffers[i].Reset();
	m_depthStencilBuffer.Reset();

	// Resize the swap chain.
	ThrowIfFailed(m_swapChain->ResizeBuffers(m_config.SwapChainBufferCount, width, height, m_config.BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_curBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart()); 
	for (UINT i = 0; i < m_config.SwapChainBufferCount; i++)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapChainBuffers[i])));
		m_device->CreateRenderTargetView(m_swapChainBuffers[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_info.DescSizes.RTV);
	}
	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_config.DepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES bufferProps{};
	bufferProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	bufferProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	bufferProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	bufferProps.CreationNodeMask = 1;
	bufferProps.VisibleNodeMask = 1;
	ThrowIfFailed(m_device->CreateCommittedResource(&bufferProps, D3D12_HEAP_FLAG_NONE,	&depthStencilDesc, D3D12_RESOURCE_STATE_COMMON,	&optClear, IID_PPV_ARGS(m_depthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = m_config.DepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	// Transition the resource from its initial state to be used as a depth buffer.
	D3D12_RESOURCE_BARRIER b{};
	b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	b.Transition.pResource = m_depthStencilBuffer.Get();
	b.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	b.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	b.Transition.Subresource = 0xffffffff;
	b.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	m_cmdList->ResourceBarrier(1, &b);

	// Execute the resize commands.
	m_cmdList->Close();
	ID3D12CommandList* cmdsLists[] = { m_cmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = static_cast<float>(width);
	m_viewport.Height = static_cast<float>(height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_scissorRect = { 0, 0, width, height};
}

void RRenderer::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	m_fenceValue++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(m_cmdQueue->Signal(m_fence.Get(), m_fenceValue));

	// Wait until the GPU has completed commands up to this fence point.
	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

ID3D12Resource* RRenderer::CurrentBackBuffer() const
{
	return m_swapChainBuffers[m_curBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE RRenderer::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_curBackBuffer,
		m_info.DescSizes.RTV);
}

D3D12_CPU_DESCRIPTOR_HANDLE RRenderer::DepthStencilView() const
{
	return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}


ComPtr<ID3DBlob> SCompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	

	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors = nullptr;
	HRESULT hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	ThrowIfFailed(hr);

	return byteCode;
}
