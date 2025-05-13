#pragma once

#include "../dx/include/d3d12.h"
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "../dx/include/d3dx12/d3dx12.h"

#include <wrl.h>
#include <memory>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace JUCore
{
	class Graphics {
	public:

		void DX12Initialize(UINT w, UINT h, HWND handler);
		void DX12ConfigLoad();
		void DX12Render();
		void DX12Destroy();


		static Graphics& get();

	private:
		Graphics();

		void MoveToNextFrame();
		void WaitForGpu();

		void PopulateCommandList();

	private:

		static std::unique_ptr<Graphics> mInstance;
		static const UINT frameCount = 2;

		float m_aspectRatio;

		struct Vertex
		{
			XMFLOAT3 position;
			XMFLOAT4 color;
		};

		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;

		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12Resource> m_renderTargets[frameCount];
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;

		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12PipelineState> m_pipelineState;

		ComPtr<ID3D12GraphicsCommandList> m_commandList;

		ComPtr<ID3D12Resource> m_vertexBuffer;

		ComPtr<ID3D12Resource> m_vertexUpload;
		ComPtr<ID3D12Resource> m_vertexDefault;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		// sync values
		UINT m_frameIndex;
		HANDLE m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValues[frameCount];

		UINT m_rtvDescriptorSize;
		//UINT frameIndex;
	};
}

