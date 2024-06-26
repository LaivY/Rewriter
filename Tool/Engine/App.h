﻿#pragma once

class Timer;

class App : public TSingleton<App>
{
public:
	App(HINSTANCE hInstance);
	~App();

	void Run();

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void InitWindow();
	void InitDirectX();
	void InitImGui();
	void InitApp();

	void OnResize(int width, int height);

	void Update();
	void Render();

	void WaitPrevFrame();

	void RenderImGui();
	void RenderImGuiMainDockSpace();
	void RenderImGuiConsole();

public:
	// Window
	static constexpr auto TITLE_NAME{ L"RW Engine 1.0v" };
	HINSTANCE m_hInstance;
	HWND hWnd;
	bool m_isActive;
	std::pair<int, int> size;

	// DirectX
	static constexpr auto FRAME_COUNT{ 3 };
	ComPtr<IDXGIFactory4> m_factory;
	ComPtr<ID3D12Device> m_d3dDevice;
	ComPtr<IDXGISwapChain3>	m_swapChain;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FRAME_COUNT];
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
	ComPtr<ID3D12DescriptorHeap> m_srvDescHeap;
	ComPtr<ID3D12Fence> m_fence;
	UINT m_cbvSrvUavDescriptorIncrementSize;
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValues[FRAME_COUNT];
	UINT m_rtvDescriptorSize;

	std::unique_ptr<Timer> m_timer;
};