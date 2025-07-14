#include "GeometryServer.h"

namespace JUCore
{
	void GeometryServer::createCopyCmdQueue(ComPtr<ID3D12Device> device)
	{
		// command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc_c = {};
		queueDesc_c.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc_c.Type = D3D12_COMMAND_LIST_TYPE_COPY;

		device->CreateCommandQueue(&queueDesc_c, IID_PPV_ARGS(&m_copyQueue));
	}

	void GeometryServer::createCopyCmdAllocator(ComPtr<ID3D12Device> device)
	{
		ID3D12CommandAllocator* copyAllocator = nullptr;
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&copyAllocator));
		m_copyCmdAllocator.Attach(copyAllocator);
	}

	void GeometryServer::createCopyCmdList(ComPtr<ID3D12Device> device)
	{
		device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_copyCmdAllocator.Get(), nullptr, IID_PPV_ARGS(&m_copyCmdList));
		m_copyCmdList->Close();
	}
}