#pragma once
#include "..\dx\include\d3d12.h"
//#include <dxgi1_6.h>
#include "..\dx\include\d3dx12\d3dx12.h"

//using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace JUCore
{
	class GeometryServer
	{
	public:
		GeometryServer();

		void createCopyCmdQueue(ComPtr<ID3D12Device> device);
		void createCopyCmdAllocator(ComPtr<ID3D12Device> device);
		void createCopyCmdList(ComPtr<ID3D12Device> device);

		void createCopyFence(ComPtr<ID3D12Device> device);
		void createCopyEvent(ComPtr<ID3D12Device> device);

		void updateGeometry(float aspectRatio, 
			ComPtr<ID3D12Resource> vertexUpload, ComPtr<ID3D12Resource> vertexDefault);

		void WaitForCopyFence();

		void Destroy();

	private:

		// Copy queue
		ComPtr<ID3D12CommandQueue> m_copyQueue;
		ComPtr<ID3D12GraphicsCommandList> m_copyCmdList;
		ComPtr<ID3D12CommandAllocator> m_copyCmdAllocator;

		HANDLE m_fenceEventCopy;
		ComPtr<ID3D12Fence> m_fenceCopy;
		UINT64 m_fenceValueCopy;
	};
}

