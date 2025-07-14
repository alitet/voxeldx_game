#pragma once

#include "..\dx\include\d3d12.h"
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "..\dx\include\d3dx12\d3dx12.h"
#include "..\libs\D3D12MemAlloc.h"

#include "GeometryServer.h"

#include <wrl.h>
#include <memory>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

//namespace D3D12MA {
//	class Allocation;
//	class Allocator;
//}

namespace JUCore
{
	constexpr unsigned char FRAME_COUNT = 2;

	class Graphics {
	public:
		Graphics();

		void DX12Initialize(UINT w, UINT h, HWND handler);
		void DX12ConfigLoad();
		void DX12Render();
		void DX12Destroy();

		void KeyUp(uint8_t key);
		void KeyDn(uint8_t key);

		//static Graphics& get();

	private:

		//void MoveToNextFrame();
		//void WaitForGpu();

		void WaitForFrame(size_t frameIndex);
		void WaitGPUIdle(size_t frameIndex);

		void PopulateCommandList();

		//void PreFillCL();
		void PostFillCL();

	private:

		//static std::unique_ptr<Graphics> mInstance;
		//constexpr UINT frameCount = 2;

		bool needRefresh = true;
		bool needUpdate = false;

		float m_aspectRatio;

		struct Vertex
		{
			XMFLOAT3 position;
			XMFLOAT4 color;
		};

		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;

		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_graphicsQueue;

		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT];
		ComPtr<ID3D12CommandAllocator> m_commandAllocator[FRAME_COUNT];

		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12PipelineState> m_pipelineState;

		ComPtr<ID3D12GraphicsCommandList> m_commandList;


		GeometryServer m_geometryServer;

		//// Copy queue
		//ComPtr<ID3D12CommandQueue> m_copyQueue;
		//ComPtr<ID3D12GraphicsCommandList> m_copyCmdList;
		//ComPtr<ID3D12CommandAllocator> m_copyCmdAllocator;


		//ComPtr<ID3D12Resource> m_vertexBuffer;

		ComPtr<ID3D12Resource> m_vertexUpload;
		ComPtr<ID3D12Resource> m_vertexDefault;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	  ComPtr<D3D12MA::Allocator> m_allocator;
		D3D12MA::Allocation* m_vertexBufferAllocation;
		D3D12MA::Allocation* m_vertexUploadAllocation;
		//D3D12MA::Allocation* m_indexBufferAllocation;


		// sync values
		UINT m_frameIndex;

		HANDLE m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence[FRAME_COUNT];
		UINT64 m_fenceValues[FRAME_COUNT];

		HANDLE m_fenceEventCopy;
		ComPtr<ID3D12Fence> m_fenceCopy;
		UINT64 m_fenceValueCopy;

		UINT m_rtvDescriptorSize;
		//UINT frameIndex;
	};
}

