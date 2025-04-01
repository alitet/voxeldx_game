#pragma once
//#include "include/d3dx12.h"
#include <wrl.h>
using Microsoft::WRL::ComPtr;

struct ID3D12Device;
struct ID3D12CommandQueue;
struct IDXGISwapChain3;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
struct ID3D12CommandAllocator;

namespace JUCore
{
	class Graphics {
	public:
		Graphics() = default;

		static void DX12Initialize(UINT w, UINT h, HWND handler);

	private:
		static const UINT frameCount = 2;

		static ComPtr<ID3D12Device> m_device;
		static ComPtr<ID3D12CommandQueue> m_commandQueue;
		static ComPtr<IDXGISwapChain3> m_swapChain;
		static ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		static ComPtr<ID3D12Resource> m_renderTargets[frameCount];
		static ComPtr<ID3D12CommandAllocator> m_commandAllocator;

		static UINT m_rtvDescriptorSize;
		static UINT frameIndex;
	};
}

