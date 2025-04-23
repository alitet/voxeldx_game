#pragma once

#include "../dx/include/d3d12.h"
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "../dx/include/d3dx12/d3dx12.h"

#include <wrl.h>
#include <memory>

using Microsoft::WRL::ComPtr;

namespace JUCore
{
	class Graphics {
	public:

		void DX12Initialize(UINT w, UINT h, HWND handler);
		void DX12ConfigLoad();

		static std::unique_ptr<Graphics> get() { return std::move(mInstance); }

	private:
		Graphics() = default;

		void MoveToNextFrame();
		void WaitForGpu();


	private:

		static std::unique_ptr<Graphics> mInstance;
		static const UINT frameCount = 2;

		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12Resource> m_renderTargets[frameCount];
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;

		ComPtr<ID3D12RootSignature> m_rootSignature;




		UINT m_rtvDescriptorSize;
		UINT frameIndex;
	};
}

