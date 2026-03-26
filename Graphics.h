#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <string>
#include <wrl/client.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace Graphics
{
	// --- CONSTANTS ---
	const unsigned int NumBackBuffers = 2;
	// Maximum number of constant buffers, assuming each buffer
	// is 256 bytes or less. Larger buffers are fine, but will
	// result in fewer buffers in use at any time.
	const unsigned int MaxConstantBuffers = 1000;
	// Maximum number of texture descriptors (SRVs) we can have.
	const unsigned int MaxTextureDescriptors = 100;

	// --- GLOBAL VARS ---

	// Primary D3D12 API Objects
	inline Microsoft::WRL::ComPtr<ID3D12Device>		Device;
	inline Microsoft::WRL::ComPtr<IDXGISwapChain>	SwapChain;

	// Command submission
	inline Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		CommandAllocators[NumBackBuffers];
	inline Microsoft::WRL::ComPtr<ID3D12CommandQueue>			CommandQueue;
	inline Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	CommandList;

	// Rendering Buffers & Descriptors
	inline Microsoft::WRL::ComPtr<ID3D12Resource>		BackBuffers[NumBackBuffers];
	inline Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	RTVHeap;
	inline D3D12_CPU_DESCRIPTOR_HANDLE					RTVHandles[NumBackBuffers]{}; // {} makes sure this is initialized empty!

	inline Microsoft::WRL::ComPtr<ID3D12Resource>		DepthBuffer;
	inline Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	DSVHeap;
	inline D3D12_CPU_DESCRIPTOR_HANDLE					DSVHandle{}; // {} makes sure this is initialized empty!

	// Constant buffers
	inline Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CBVSRVDescriptorHeap;
	inline Microsoft::WRL::ComPtr<ID3D12Resource> CBUploadHeap;

	// Basic CPU/GPU synchronization
	inline Microsoft::WRL::ComPtr<ID3D12Fence>	WaitFence;
	inline HANDLE								WaitFenceEvent = 0;
	inline UINT64								WaitFenceCounter = 0;

	// Multi-frame synchronization
	inline Microsoft::WRL::ComPtr<ID3D12Fence>	FrameSyncFence;
	inline HANDLE								FrameSyncEvent = 0;
	inline UINT64								FrameSyncFenceCounters[NumBackBuffers]{};

	// Debug Layer
	inline Microsoft::WRL::ComPtr<ID3D12InfoQueue> InfoQueue;

	// --- FUNCTIONS ---

	// Getters
	bool VsyncState();
	std::wstring APIName();
	unsigned int SwapChainIndex();

	// General functions
	HRESULT Initialize(unsigned int windowWidth, unsigned int windowHeight, HWND windowHandle, bool vsyncIfPossible);
	void ShutDown();
	void ResizeBuffers(unsigned int width, unsigned int height);
	void AdvanceSwapChainIndex();
	unsigned int LoadTexture(const wchar_t* file, bool generateMips = true);

	// Resource Creation
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateStaticBuffer(size_t dataStride, size_t dataCount, void* data);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer(
		UINT64 size,
		D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		UINT64 alignment = 0,
		void* data = nullptr,
		size_t datasize = 0);
	void ReserveDescriptorHeapSlot(
		D3D12_CPU_DESCRIPTOR_HANDLE* reservedCPUHandle,
		D3D12_GPU_DESCRIPTOR_HANDLE* reservedGPUHandle);
	unsigned int GetDescriptorIndex(D3D12_GPU_DESCRIPTOR_HANDLE handle);

	// Ring buffer management
	D3D12_GPU_DESCRIPTOR_HANDLE FillNextConstantBufferAndGetGPUDescriptorHandle(void* data, unsigned int dataSizeInBytes);

	// Debug Layer
	void PrintDebugMessages();
	void ResetAllocatorAndCommandList(unsigned int bufferIndex);
	void CloseAndExecuteCommandList();
	void WaitForGPU();
}