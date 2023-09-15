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
	float speedMultiplier = 0.1f;
	glm::vec3 delta{0};
	if (INPUT->GetKeyState(InputCode::W) > InputState::KeyUp)
		delta += m_camera->GetDirection();
	if (INPUT->GetKeyState(InputCode::S) > InputState::KeyUp)
		delta += -m_camera->GetDirection();
	if (INPUT->GetKeyState(InputCode::A) > InputState::KeyUp)
		delta += -m_camera->GetRight();
	if (INPUT->GetKeyState(InputCode::D) > InputState::KeyUp)
		delta += m_camera->GetRight();

	if (delta.x || delta.y || delta.z)
	{
		auto x =  glm::normalize(delta) * speedMultiplier;
		m_camera->SetPosition(m_camera->GetPosition() + x);
		m_camera->SetLookAt(m_camera->GetLookAt() + x);
	}

	if (m_camera->RequiresUpdate())
	{
		m_camera->Update();
		m_cameraConstantBuffer->CopyData(0, m_camera->GetShaderData());
		m_rtConstants.AccumlateSamples = false;
		m_rtConstants.ResetOutput = true;
		m_rtConstants.AccumulatedSamples = 1;
	}
	std::random_device r;
	std::default_random_engine e1(r());
	std::uniform_int_distribution<unsigned int> uniform_dist(0, 0xffffffff);

	// Random seed is set in the constructor
	m_rtConstants.RandSeed = uniform_dist(e1);

	m_rtConstantBuffer->CopyData(0, m_rtConstants);
	

	Draw();

	m_rtConstants.ResetOutput = false;
	m_rtConstants.AccumlateSamples = m_rtConstants.AccumulatedSamples < m_currentMaxSamples;
	if (!(m_rtConstants.AccumulatedSamples - (m_currentMaxSamples-1)))
	{
		// This actually gets displayed 1 samples before the currentMax
		CORE_INFO("Accumulated {} samples!", m_currentMaxSamples);
	}
	m_rtConstants.AccumulatedSamples += m_rtConstants.AccumlateSamples;
}

void RRenderer::Draw()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView(),
								dsv = m_dsvHeap->GetCPUDescriptorHandleForHeapStart(),
								uav = m_srvHeap->GetCPUDescriptorHandleForHeapStart();

	ThrowIfFailed(m_cmdAlloc->Reset());
	ThrowIfFailed(m_cmdList->Reset(m_cmdAlloc.Get(), m_computePSO.Get()));

	D3D12_RESOURCE_BARRIER bs[2]{
		CD3DX12_RESOURCE_BARRIER::Transition(m_swapChainBuffers[m_curBackBuffer].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST),
		CD3DX12_RESOURCE_BARRIER::Transition(m_computeOutputBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	};

	// Set Viewport
	m_cmdList->RSSetViewports(1, &m_viewport);
	m_cmdList->RSSetScissorRects(1, &m_scissorRect);

	// Transition Resources
	m_cmdList->ResourceBarrier(2, bs);

	// Initial Graphics stuff
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_cmdList->SetComputeRootSignature(m_computeRootSig.Get());

	ComPtr<ID3D12DescriptorHeap> heaps[1] = {m_srvHeap};
	m_cmdList->SetDescriptorHeaps(1, heaps->GetAddressOf());

	// Dispatch compute shader
	D3D12_GPU_DESCRIPTOR_HANDLE descH = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	m_cmdList->SetComputeRootDescriptorTable(0, descH);

	unsigned int xGroups = static_cast<unsigned int>(ceilf(m_clientWidth / 256.0f));
	m_cmdList->Dispatch(xGroups, m_clientHeight, 1);

	// Ready output texture as copy source and backbuffer from copy dest to present state (the transitions will take place, respectively and sequentially, before and after the resource has been copied)
	bs[0] = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
	bs[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_computeOutputBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

	// Transition 
	m_cmdList->ResourceBarrier(1, &bs[1]);
	// Copy 
	m_cmdList->CopyResource(m_swapChainBuffers[m_curBackBuffer].Get(), m_computeOutputBuffer.Get());
	m_cmdList->ResourceBarrier(1, &bs[0]);

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

	m_camera->SetViewport({ m_clientWidth, m_clientHeight }, SWindow::GetInstance()->GetAspectRatio());
	return false;
}

bool RRenderer::OnKeyDown(IEvent* e)
{
	KeyPressedEvent* event = dynamic_cast<KeyPressedEvent*>(e);
	switch (event->GetKeyCode())
	{
		case KeyCode::I:
		{
			if (m_rtConstants.MaxRayBounces < 10)
			{
				m_rtConstants.MaxRayBounces += 1;
				m_rtConstants.AccumlateSamples = false;
				m_rtConstants.ResetOutput = true;
				m_rtConstants.AccumulatedSamples = 1;
			}

			CORE_INFO("Ray Bounces: {}", m_rtConstants.MaxRayBounces);
			break;
		}
		case KeyCode::K:
		{
			if (m_rtConstants.MaxRayBounces)
			{
				m_rtConstants.MaxRayBounces -= 1;
				m_rtConstants.AccumlateSamples = false;
				m_rtConstants.ResetOutput = true;
				m_rtConstants.AccumulatedSamples = 1;
			}
				
			CORE_INFO("Ray Bounces: {}", m_rtConstants.MaxRayBounces);
			break;
		}
		case KeyCode::Enter:
		{
			// List All Camera Information in console
			CORE_INFO("{}", m_camera->GetInfo());
			break;
		}
		default:
			break;
	}

	return false;
}

bool RRenderer::OnMouseWheel(IEvent* e)
{
	MouseScrolledEvent* scrollEvent = dynamic_cast<MouseScrolledEvent*>(e);
	float delta = scrollEvent->GetScrollDelta() / 120.f; 

	switch (m_mouseWheelMode)
	{
		case 0:
		{
			float newFov = m_camera->GetFov() - delta * 5; // Negated based on my control preferences
			if (newFov > 9 && newFov < 101)
				m_camera->SetFov(newFov);


			break;
		}
		case 1:
		{
			float newFocalDist = m_camera->GetFocalDist() + delta * 1.f;
			if (newFocalDist > .1f)
				m_camera->SetFocalDist(newFocalDist);
			break;
		}
		case 2:
		{
			float newDefocusAngle = m_camera->GetDefocusAngle() + delta * 0.1f;
			if (newDefocusAngle > 0)
				m_camera->SetDefocusAngle(newDefocusAngle);
			break;
		}
	}
	return false;
}


bool RRenderer::OnMouseDown(IEvent*e)
{
	MouseButtonPressedEvent* mouseEvent = dynamic_cast<MouseButtonPressedEvent*>(e);
	
	std::array<std::string, 3> mwheelModes = {"FOV", "FocalDist", "DefocusAngle"};
	auto button = mouseEvent->GetMouseButton();
	switch (button)
	{
		case MouseCode::MB2:
		{
			m_mouseWheelMode = (m_mouseWheelMode + 1) % 3;
			CORE_INFO("Mouse Wheel Mode: {}", mwheelModes[m_mouseWheelMode]);
			break;
		}
	}
	
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
	

	m_cmdList->Reset(m_cmdAlloc.Get(), nullptr);
	CreateObjects();
	CreateInputLayoutAndShaders();
	CreateRootSignatureAndPSOs();
	m_cmdList->Close();

	ID3D12CommandList* cmdLists[] = { m_cmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(1, cmdLists);

	// Wait for Intialization
	FlushCommandQueue();

	// Initialize Camera
	m_camera = std::make_unique<RTCamera>(
		glm::vec3{ 13.f, 2.f, -3.f },
		glm::vec3{ 0.f, 0.f, 0.f },
		glm::vec3{ 0.f, 1.f, 0.f },
		glm::ivec2{ m_clientWidth, m_clientHeight},
		SWindow::GetInstance()->GetAspectRatio(),
		20.f,
		10.f,
		0.6f);

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
	D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {}, dsvDesc = {}, srvDesc{};
	rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDesc.NumDescriptors = m_config.SwapChainBufferCount;

	dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDesc.NumDescriptors = 1;

	srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvDesc.NumDescriptors = 6;

	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&m_rtvHeap)));
	ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvDesc, IID_PPV_ARGS(&m_dsvHeap)));
	ThrowIfFailed(m_device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&m_srvHeap)));
}

void RRenderer::CreateInputLayoutAndShaders()
{
	HRESULT hr = S_OK;

	m_vsByteCode = SCompileShader(L"Assets\\Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	m_psByteCode = SCompileShader(L"Assets\\Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");
	m_csByteCode = SCompileShader(L"Assets\\Shaders\\rtiaw.hlsl", nullptr, "CS", "cs_5_0");


	m_inputLayout =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
}

void RRenderer::CreateObjects()
{
	auto aspectRatio = SWindow::GetInstance()->GetAspectRatio();
	std::vector<Vertex> vertices(3);
	vertices =
	{
		Vertex({ DirectX::XMFLOAT3(0.0f, 0.25f * aspectRatio, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Red)}),
		Vertex({ DirectX::XMFLOAT3(0.25f, -0.25f * aspectRatio, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f)}),
		Vertex({ DirectX::XMFLOAT3(-0.25f, -0.25f * aspectRatio, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Blue)}),
	};

	std::vector<std::uint16_t> indices =
	{
		0,1,2
	};
	m_mesh = CreateScope<Mesh>(m_device.Get(), m_cmdList.Get(), vertices, indices);
	m_mesh->m_name = "TriangleMesh";

	SubMesh sm;
	sm.BaseVertexLocation = 0;
	sm.StartIndexLocation = 0;
	sm.IndexCount = static_cast<uint32_t>(indices.size());
	m_mesh->m_subMeshes["Triangle"] = sm;



	// Random stuff / refactor and move to its own function/file (also remove from Update function)
	
	std::random_device r;
	std::default_random_engine e1(r());
	std::uniform_int_distribution<unsigned int> uniform_dist(0, 0xffffffff);
	auto randomFloat = [&e1, &uniform_dist]() {
		return uniform_dist(e1) / (float)0xffffffff;
	};

	// RT SPHERES

	m_materials.push_back({{ 0.5f, 0.5f, 0.5f }, MTType::Diffuse, .0f});	// Ground material
	m_spheres.push_back({ {0.f, -1000.f, 0.f}, 1000.f, 0});
	
	RTMaterial sphereMaterial;
	glm::vec3 center;
	int size = 6;

	for (int a = -size; a < size; a++)
	{
		for (int b = -size; b < size; b++)
		{
			float mat = randomFloat();
			center = { a + 0.9f * randomFloat(), 0.2f, b + 0.9f * randomFloat() };

			if (((a < -4 || a > 4) || (b > 1 || b < -1)) && (center - glm::vec3(4.f, 0.2f, 0.f)).length() > 0.9f)
			{
				sphereMaterial.Albedo = glm::vec3(randomFloat(), randomFloat(), randomFloat());
				sphereMaterial.Roughness = randomFloat();
				if (mat < 0.75f)
				{
					sphereMaterial.Type = Diffuse;
				}else if (mat < 0.90)
				{
					// metal
					sphereMaterial.Type = Metal;
				}
				else
				{
					sphereMaterial.Type = Dielectric;
				}
				m_materials.push_back(sphereMaterial);
				m_spheres.push_back({ center, 0.2f, static_cast<uint32_t>(m_materials.size() - 1) });
			}
		}
	}
	
	sphereMaterial.Albedo = glm::vec3(randomFloat(), randomFloat(), randomFloat());
	sphereMaterial.Roughness = randomFloat();
	sphereMaterial.Type = Dielectric;
	m_materials.push_back(sphereMaterial);
	m_spheres.push_back({ {0.f, 1.f, 0.f}, 1.0f, static_cast<uint32_t>(m_materials.size() - 1) });

	
	sphereMaterial.Albedo = glm::vec3(randomFloat(), randomFloat(), randomFloat());
	sphereMaterial.Roughness = 0.0f;
	sphereMaterial.Type = Metal;
	m_materials.push_back(sphereMaterial);
	m_spheres.push_back({ {4.f, 1.f, 0.f}, 1.0f, static_cast<uint32_t>(m_materials.size() - 1) });
	

	sphereMaterial.Albedo = glm::vec3(randomFloat(), randomFloat(), randomFloat());
	sphereMaterial.Roughness = randomFloat();
	sphereMaterial.Type = Diffuse;
	m_materials.push_back(sphereMaterial);
	m_spheres.push_back({ {-4.f, 1.f, 0.f}, 1.0f, static_cast<uint32_t>(m_materials.size() - 1) });


	m_sphereBuffer = CreateDefaultBuffer(m_device.Get(), m_cmdList.Get(), m_spheres.data(), m_spheres.size() * sizeof(RTSphere), m_sphereUploadBuffer);
	m_materialBuffer = CreateDefaultBuffer(m_device.Get(), m_cmdList.Get(), m_materials.data(), m_spheres.size() * sizeof(RTMaterial), m_materialUploadBuffer);
	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_buffer_srv
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.StructureByteStride = sizeof(RTSphere);
	srvDesc.Buffer.NumElements = static_cast<UINT>(m_spheres.size());
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	// Shader Resource View for the Buffer that contains the sphere data is the 4 and last descriptor in the srv descriptor heap
	CD3DX12_CPU_DESCRIPTOR_HANDLE descHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart()); 
	descHandle.Offset(4, m_info.DescSizes.CBV);
	m_device->CreateShaderResourceView(m_sphereBuffer.Get(), &srvDesc, descHandle);
	
	srvDesc.Buffer.StructureByteStride = sizeof(RTMaterial);
	srvDesc.Buffer.NumElements = static_cast<UINT>(m_materials.size());
	descHandle.Offset(1, m_info.DescSizes.SRV);
	m_device->CreateShaderResourceView(m_materialBuffer.Get(), &srvDesc, descHandle);

}

void RRenderer::CreateRootSignatureAndPSOs()
{
	// ROOT SIGNATURE
	D3D12_ROOT_SIGNATURE_DESC rootDesc= {};
	rootDesc.NumParameters = 0;
	rootDesc.pParameters = nullptr;
	rootDesc.NumStaticSamplers = 0;
	rootDesc.pStaticSamplers = nullptr;
	rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob>	rootSerializerBlob = nullptr,
						errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, rootSerializerBlob.GetAddressOf(), errorBlob.GetAddressOf());
	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	ThrowIfFailed(hr);
	ThrowIfFailed(m_device->CreateRootSignature(NULL, rootSerializerBlob->GetBufferPointer(), rootSerializerBlob->GetBufferSize(), IID_PPV_ARGS(&m_opaqueRootSig)));

	// COMPUTE SHADER ROOT SIGNATURE
	std::array<CD3DX12_DESCRIPTOR_RANGE, 3> ranges;
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0); // Final Output Color Texture and Accumulated Color Texture
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0); // RTFrame and Camera Constants
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0); // Geometry Data (For now: only Spheres, just the position and the radius) and Materials

	CD3DX12_ROOT_PARAMETER params[1];
	params[0].InitAsDescriptorTable(static_cast<UINT>(ranges.size()), &ranges[0]);
	CD3DX12_ROOT_SIGNATURE_DESC crDesc(1, params, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	rootSerializerBlob = nullptr,
	errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&crDesc, D3D_ROOT_SIGNATURE_VERSION_1, rootSerializerBlob.GetAddressOf(), errorBlob.GetAddressOf());
	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	ThrowIfFailed(hr);
	ThrowIfFailed(m_device->CreateRootSignature(NULL, rootSerializerBlob->GetBufferPointer(), rootSerializerBlob->GetBufferSize(), IID_PPV_ARGS(&m_computeRootSig)));
	





	// PSOs
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
	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_opaquePSO)));


	D3D12_COMPUTE_PIPELINE_STATE_DESC cmptDesc{};
	cmptDesc.CS =
	{
		reinterpret_cast<BYTE*>(m_csByteCode->GetBufferPointer()),
		m_csByteCode->GetBufferSize()
	};
	cmptDesc.pRootSignature = m_computeRootSig.Get();
	ThrowIfFailed(m_device->CreateComputePipelineState(&cmptDesc, IID_PPV_ARGS(&m_computePSO)));

	cmptDesc.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
	cmptDesc.pRootSignature = m_computeRootSig.Get();

}

void RRenderer::CreateConstantBuffers()
{
	m_rtConstantBuffer = CreateScope<UploadBuffer<RTConstants>>(m_device.Get(), 1, true);
	m_cameraConstantBuffer = CreateScope<UploadBuffer<RTCameraSD>>(m_device.Get(), 1, true);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = m_rtConstantBuffer->Resource()->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = CalcConstantBufferByteSize(sizeof(RTConstants));

	CD3DX12_CPU_DESCRIPTOR_HANDLE descHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	descHandle.Offset(2, m_info.DescSizes.UAV);

	// Create Descriptor for Frame Constants
	m_device->CreateConstantBufferView(&cbvDesc, descHandle);
	
	cbvDesc.BufferLocation = m_cameraConstantBuffer->Resource()->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = CalcConstantBufferByteSize(sizeof(RTCamera));
	descHandle.Offset(1, m_info.DescSizes.CBV);
	// Create Descriptor for Camera Constants
	m_device->CreateConstantBufferView(&cbvDesc, descHandle);

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
	m_computeOutputBuffer.Reset();
	m_computeAccumulateBuffer.Reset();

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
	D3D12_RESOURCE_DESC depthStencilDesc{};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality =0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear{};
	optClear.Format = m_config.DepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES textureProps{};
	textureProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	textureProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	textureProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	textureProps.CreationNodeMask = 1;
	textureProps.VisibleNodeMask = 1;
	ThrowIfFailed(m_device->CreateCommittedResource(
		&textureProps,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(m_depthStencilBuffer.GetAddressOf())));

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
	b.Transition.StateBefore= D3D12_RESOURCE_STATE_COMMON;
	b.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	b.Transition.Subresource = 0xffffffff;
	b.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	m_cmdList->ResourceBarrier(1, &b);


	// COMPUTE STUFF
	D3D12_RESOURCE_DESC rtRWTextureDesc{};
	rtRWTextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rtRWTextureDesc.Alignment = 0;
	rtRWTextureDesc.Width = width;
	rtRWTextureDesc.Height = height;
	rtRWTextureDesc.DepthOrArraySize = 1;
	rtRWTextureDesc.MipLevels = 1;
	rtRWTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtRWTextureDesc.SampleDesc.Count = 1;
	rtRWTextureDesc.SampleDesc.Quality = 0;
	rtRWTextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rtRWTextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;


	ThrowIfFailed(m_device->CreateCommittedResource(&textureProps, D3D12_HEAP_FLAG_NONE, &rtRWTextureDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(m_computeOutputBuffer.GetAddressOf())));
	// ACCUMULATED COLOR BUFFER 
	rtRWTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; 
	ThrowIfFailed(m_device->CreateCommittedResource(&textureProps, D3D12_HEAP_FLAG_NONE, &rtRWTextureDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(m_computeAccumulateBuffer.GetAddressOf())));
	
	
	// CREATE DESCRIPTORS FOR COMPUTE RESOURCES
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDH = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	m_device->CreateUnorderedAccessView(m_computeOutputBuffer.Get(), nullptr, &uavDesc, cpuDH);
	uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	cpuDH.Offset(1, m_info.DescSizes.UAV);
	m_device->CreateUnorderedAccessView(m_computeAccumulateBuffer.Get(), nullptr, &uavDesc, cpuDH);

	// RESOURCE TRANSITIONS
	D3D12_RESOURCE_BARRIER uavBar[2] = { 
		CD3DX12_RESOURCE_BARRIER::Transition(m_computeOutputBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(m_computeAccumulateBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS) 
	};
	m_cmdList->ResourceBarrier(2, uavBar);


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

D3D12_CPU_DESCRIPTOR_HANDLE RRenderer::ComputeTextureUAView() const
{
	return m_srvHeap->GetCPUDescriptorHandleForHeapStart();
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