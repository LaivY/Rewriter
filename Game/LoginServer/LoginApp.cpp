﻿#include "Stdafx.h"
#include "DBThread.h"
#include "IOCPThread.h"
#include "LoginApp.h"

LoginApp::LoginApp() :
	m_hWnd{ NULL },
	m_wndSize{ 600, 400 },
	m_cbvSrvUavDescriptorIncrementSize{ 0 },
	m_frameIndex{ 0 },
	m_fenceEvent{ 0 },
	m_fenceValues{ 0, },
	m_rtvDescriptorSize{ 0 },
	m_isDxInit{ false }
{
}

void LoginApp::OnCreate()
{
	InitWnd();
	InitDirectX();
	InitImgui();

	DBThread::Instantiate();
	IOCPThread::Instantiate();
}

void LoginApp::Run()
{
	MSG msg{};
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Update();
			Render();
		}
	}
}

LRESULT LoginApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	LoginApp* app{ reinterpret_cast<LoginApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)) };
	switch (message)
	{
	case WM_NCCREATE:
	{
		LPCREATESTRUCT pcs{ reinterpret_cast<LPCREATESTRUCT>(lParam) };
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pcs->lpCreateParams));
		return 1;
	}
	case WM_SIZE:
		app->OnResize({ static_cast<int>(LOWORD(lParam)), static_cast<int>(HIWORD(lParam)) });
		break;
	case WM_DESTROY:
		app->OnDestroy();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void LoginApp::InitWnd()
{
	WNDCLASSEX wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = HINST_THISCOMPONENT;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = TEXT("Login Server");
	RegisterClassEx(&wcex);

	RECT rect{ 0, 0, m_wndSize.x, m_wndSize.y };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	m_hWnd = CreateWindow(
		wcex.lpszClassName,
		TEXT("Login Server"),
		WS_OVERLAPPED | WS_SYSMENU | WS_BORDER | WS_THICKFRAME,
		//WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		wcex.hInstance,
		this
	);

	ShowWindow(m_hWnd, SW_SHOWNORMAL);
	UpdateWindow(m_hWnd);
}

void LoginApp::InitDirectX()
{
	CreateFactory();
	CreateDevice();
	CreateCommandQueue();
	CreateSwapChain();
	CreateRtvDescriptorHeap();
	CreateSrvDescriptorHeap();
	CreateRenderTargetView();
	CreateCommandList();
	CreateFence();
	m_isDxInit = true;
}

void LoginApp::InitImgui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.Fonts->AddFontFromFileTTF("C:/Dev/ReWriter/Client/Data/morris9.ttf", 12.0f, nullptr, io.Fonts->GetGlyphRangesKorean());
	ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_None;
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX12_Init(
		m_d3dDevice.Get(),
		FRAME_COUNT,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		m_srvDescHeap.Get(),
		m_srvDescHeap->GetCPUDescriptorHandleForHeapStart(),
		m_srvDescHeap->GetGPUDescriptorHandleForHeapStart());
}

void LoginApp::CreateFactory()
{
	UINT dxgiFactoryFlags{ 0 };
#ifdef _DEBUG
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory));
}

void LoginApp::CreateDevice()
{
	ComPtr<IDXGIAdapter1> hardwareAdapter;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_factory->EnumAdapters1(i, &hardwareAdapter); ++i)
	{
		DXGI_ADAPTER_DESC1 adapterDesc{};
		hardwareAdapter->GetDesc1(&adapterDesc);
		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice)))) break;
	}
	if (!m_d3dDevice)
	{
		m_factory->EnumWarpAdapter(IID_PPV_ARGS(&hardwareAdapter));
		D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice));
	}
	m_cbvSrvUavDescriptorIncrementSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void LoginApp::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
}

void LoginApp::CreateSwapChain()
{
	RECT rect{};
	GetClientRect(m_hWnd, &rect);

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.BufferCount = FRAME_COUNT;
	swapChainDesc.Width = rect.right - rect.left;
	swapChainDesc.Height = rect.bottom - rect.top;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	m_factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		m_hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	);

	swapChain.As(&m_swapChain);
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// ALT + ENTER 금지
	m_factory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
}

void LoginApp::CreateRtvDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = FRAME_COUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = NULL;
	m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
	m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void LoginApp::CreateSrvDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NodeMask = NULL;
	m_d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvDescHeap));
}

void LoginApp::CreateRenderTargetView()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ m_rtvHeap->GetCPUDescriptorHandleForHeapStart() };
	for (UINT i = 0; i < FRAME_COUNT; ++i)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
		m_d3dDevice->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(m_rtvDescriptorSize);

		m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
	}
}

void LoginApp::CreateCommandList()
{
	m_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), nullptr, IID_PPV_ARGS(&m_commandList));
	m_commandList->Close();
}

void LoginApp::CreateFence()
{
	m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	m_fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	++m_fenceValues[m_frameIndex];
}

void LoginApp::WaitPrevFrame()
{
	const UINT64 currentFenceValue{ m_fenceValues[m_frameIndex] };
	m_commandQueue->Signal(m_fence.Get(), currentFenceValue);

	if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
	{
		m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

void LoginApp::OnResize(const INT2& size)
{
	if (!m_isDxInit)
		return;

	WaitPrevFrame();

	for (int i = 0; i < FRAME_COUNT; ++i)
	{
		m_renderTargets[i].Reset();
		m_commandAllocators[i].Reset();
		m_fenceValues[i] = m_fenceValues[m_frameIndex];
	}

	DXGI_SWAP_CHAIN_DESC desc{};
	m_swapChain->GetDesc(&desc);
	m_swapChain->ResizeBuffers(FRAME_COUNT, size.x, size.y, desc.BufferDesc.Format, desc.Flags);
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	CreateRenderTargetView();
}

void LoginApp::OnDestroy()
{
	DBThread::Destroy();
	IOCPThread::Destroy();

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void LoginApp::Update()
{

}

void LoginApp::Render()
{
	m_commandAllocators[m_frameIndex]->Reset();
	m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<int>(m_frameIndex), m_rtvDescriptorSize };
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	constexpr FLOAT clearColor[]{ 0.15625f, 0.171875f, 0.203125f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, NULL);
	m_commandList->SetDescriptorHeaps(1, m_srvDescHeap.GetAddressOf());

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	RenderBackgroundWindow();
	if (auto dbThread{ DBThread::GetInstance() })
		dbThread->Render();
	if (auto userThread{ IOCPThread::GetInstance() })
		userThread->Render();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	m_commandList->Close();
	ID3D12CommandList* ppCommandList[]{ m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandList), ppCommandList);

	m_swapChain->Present(1, 0);
	WaitPrevFrame();
}

void LoginApp::RenderBackgroundWindow()
{
	RECT rect{};
	GetClientRect(m_hWnd, std::addressof(rect));

	ImGui::Begin("Background", NULL,
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking);
	ImGui::SetWindowSize({ static_cast<float>(rect.right), static_cast<float>(rect.bottom) });
	ImGui::SetWindowPos({ 0.0f, 0.0f });
	ImGui::DockSpace(ImGui::GetID("Background"), {}, ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();
}