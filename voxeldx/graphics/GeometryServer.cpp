#include "GeometryServer.h"
#include "grapheader.h"
#include <cassert>

namespace JUCore
{
	GeometryServer::GeometryServer()
		: m_fenceEventCopy(0), m_fenceValueCopy(0)
	{
	}

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

	void GeometryServer::createCopyFence(ComPtr<ID3D12Device> device)
	{
		ID3D12Fence* fencecpy = nullptr;
		device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fencecpy));
		m_fenceCopy.Attach(fencecpy);
		m_fenceValueCopy = 0;
	}

	void GeometryServer::createCopyEvent(ComPtr<ID3D12Device> device)
	{
		m_fenceEventCopy = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		assert(m_fenceEventCopy);
	}

	void GeometryServer::updateGeometry(float aspectRatio, 
		ComPtr<ID3D12Resource> vertexUpload,
		ComPtr<ID3D12Resource> vertexDefault)
	{
		Vertex triangleVertices[] =
		{
				{ { 0.0f, -0.25f * aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
				{ { -0.25f, 0.25f * aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
				{ { 0.25f, 0.25f * aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		//const UINT vBufferSize = sizeof(triangleVertices);

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		vertexUpload->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		vertexUpload->Unmap(0, nullptr);

		auto resDFF = CD3DX12_RESOURCE_BARRIER::Transition(vertexDefault.Get(),
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		m_copyCmdList->ResourceBarrier(1, &resDFF);

		m_copyCmdList->CopyResource(vertexDefault.Get(), vertexUpload.Get());

		auto resVBB = CD3DX12_RESOURCE_BARRIER::Transition(vertexDefault.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		m_copyCmdList->ResourceBarrier(1, &resVBB);

		ID3D12CommandList* ppCommandLists[] = { m_copyCmdList.Get() };
		m_copyQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		m_fenceValueCopy++;
		m_copyQueue->Signal(m_fenceCopy.Get(), m_fenceValueCopy);
	}

	void GeometryServer::WaitForCopyFence()
	{
		if (m_fenceCopy->GetCompletedValue() < m_fenceValueCopy)
		{			
			m_fenceCopy->SetEventOnCompletion(m_fenceValueCopy, m_fenceEventCopy);
			WaitForSingleObject(m_fenceEventCopy, INFINITE);
		}
	}

	void GeometryServer::Destroy()
	{
		CloseHandle(m_fenceEventCopy);
	}

}