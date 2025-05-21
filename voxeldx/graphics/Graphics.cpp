#include "Graphics.h"
//#include "..\libs\D3D12MemAlloc.h"

static constexpr D3D12MA::ALLOCATOR_FLAGS g_AllocatorFlags = D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;
static const bool ENABLE_DEBUG_LAYER = true;
//static const bool ENABLE_CPU_ALLOCATION_CALLBACKS = true;
//static D3D12MA::ALLOCATION_CALLBACKS g_AllocationCallbacks = {};

namespace JUCore
{
  std::wstring GetAssetFullPath(LPCWSTR assetName)
  {
    return L"data/shaders/" + std::wstring(assetName);
  }

  Graphics::Graphics()
    : m_frameIndex(0), m_rtvDescriptorSize(0)
    , m_aspectRatio(1.f), m_fenceEvent(0)
  {
  }

  ComPtr<IDXGIAdapter1> GetHardwareAdapter(_In_ IDXGIFactory1* pFactory,
    //_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
    bool requestHighPerformanceAdapter = false)
  {
    //*ppAdapter = nullptr;
    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
      for (UINT adapterIndex = 0;
        SUCCEEDED(factory6->EnumAdapterByGpuPreference(
          adapterIndex,
          requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
          IID_PPV_ARGS(&adapter)));
          ++adapterIndex)
      {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) { continue; }

        // Check DX12 device, mock create
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr))) {
          break;
        }
      }
    }

    //*ppAdapter = adapter.Detach();
    return adapter;
  }

  // Adapted from MiniEngine
  void Graphics::DX12Initialize(UINT w, UINT h, HWND handler)
  {
    UINT dxgiFactoryFlags = 0;
    m_aspectRatio = static_cast<float>(w) / static_cast<float>(h);

    m_viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h) };
    m_scissorRect = CD3DX12_RECT{ 0, 0, static_cast<LONG>(w), static_cast<LONG>(h) };

#if defined(_DEBUG)
    {
      ComPtr<ID3D12Debug> debugController;
      if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
      }
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
    ComPtr<IDXGIAdapter1> hardwareAdapter = GetHardwareAdapter(factory.Get());

    D3D12CreateDevice(hardwareAdapter.Get(),
      D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));

    // command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

    // swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FRAME_COUNT;
    swapChainDesc.Width = w;
    swapChainDesc.Height = h;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    factory->CreateSwapChainForHwnd(
      m_commandQueue.Get(),
      handler,
      &swapChainDesc,
      nullptr,
      nullptr,
      &swapChain
    );

    // No fullscreen
    factory->MakeWindowAssociation(handler, DXGI_MWA_NO_ALT_ENTER);

    swapChain.As(&m_swapChain);
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // descriptor heaps.
    {
      // render target view (RTV) descriptor heap.
      D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
      rtvHeapDesc.NumDescriptors = FRAME_COUNT;
      rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));

      m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // frame resources.
    {
      CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

      // a RTV for each frame.
      for (UINT n = 0; n < FRAME_COUNT; n++)
      {
        m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
        m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
      }
    }

    for (int i = 0; i < FRAME_COUNT; i++)
    {
      ID3D12CommandAllocator* commandAllocator = nullptr;
      m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
      m_commandAllocator[i].Attach(commandAllocator);
    }

    {
      D3D12MA::ALLOCATOR_DESC desc = {};
      desc.Flags = g_AllocatorFlags;
      desc.pDevice = m_device.Get();
      desc.pAdapter = hardwareAdapter.Get();

      //if (ENABLE_CPU_ALLOCATION_CALLBACKS) {
      //  g_AllocationCallbacks.pAllocate = &CustomAllocate;
      //  g_AllocationCallbacks.pFree = &CustomFree;
      //  g_AllocationCallbacks.pPrivateData = CUSTOM_ALLOCATION_PRIVATE_DATA;
      //  desc.pAllocationCallbacks = &g_AllocationCallbacks;
      //}

      D3D12MA::CreateAllocator(&desc, &m_allocator);
    }

    for (int i = 0; i < FRAME_COUNT; i++)
    {
      ID3D12Fence* fence = nullptr;
      m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
      m_fence[i].Attach(fence);
      m_fenceValues[i] = 0; // set the initial g_Fences value to 0
    }

    // create a handle to a g_Fences event
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    assert(m_fenceEvent);

  }

  void Graphics::DX12ConfigLoad()
  {
    { // root sig
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

      ComPtr<ID3DBlob> signature;
      ComPtr<ID3DBlob> error;
      D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error);
      m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
    }

    // the pipeline state, that compiling and loading shaders.
    {
      ComPtr<ID3DBlob> vertexShader;
      ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
      UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
      UINT compileFlags = 0;
#endif

      D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
      D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

      // Define the vertex input layout.
      D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
      {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
          { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
      };

      // Describe and create the graphics pipeline state object (PSO).
      D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
      psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
      psoDesc.pRootSignature = m_rootSignature.Get();
      psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
      psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
      psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
      psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
      psoDesc.DepthStencilState.DepthEnable = FALSE;
      psoDesc.DepthStencilState.StencilEnable = FALSE;
      psoDesc.SampleMask = UINT_MAX;
      psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      psoDesc.NumRenderTargets = 1;
      psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
      psoDesc.SampleDesc.Count = 1;
      m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
    }

    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList));
    // cerrado porque no graba nada
    m_commandList->Close();

    // vertex buffer.
    {
      // geometry of triangle.
      Vertex triangleVertices[] =
      {
          { { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
          { { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
          { { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
      };

      const UINT vBufferSize = sizeof(triangleVertices);

      //--------------------------------------------------------------ALLOCC

      //default heap
      D3D12MA::CALLOCATION_DESC vertexBufferAllocDesc = D3D12MA::CALLOCATION_DESC{ D3D12_HEAP_TYPE_DEFAULT };
      D3D12_RESOURCE_DESC vertexBufferResourceDesc = {};
      vertexBufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      vertexBufferResourceDesc.Alignment = 0;
      vertexBufferResourceDesc.Width = vBufferSize;
      vertexBufferResourceDesc.Height = 1;
      vertexBufferResourceDesc.DepthOrArraySize = 1;
      vertexBufferResourceDesc.MipLevels = 1;
      vertexBufferResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
      vertexBufferResourceDesc.SampleDesc.Count = 1;
      vertexBufferResourceDesc.SampleDesc.Quality = 0;
      vertexBufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      vertexBufferResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

      ID3D12Resource* vertexBufferPtr;
      m_allocator->CreateResource(
        &vertexBufferAllocDesc,
        &vertexBufferResourceDesc, // resource description for a buffer
        D3D12_RESOURCE_STATE_COMMON,
        nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
        &m_vertexBufferAllocation,
        IID_PPV_ARGS(&vertexBufferPtr));
      m_vertexDefault.Attach(vertexBufferPtr);

      m_vertexDefault->SetName(L"VertexBufferResHeap");
      m_vertexBufferAllocation->SetName(L"VertexBufResHeapAlloc");

  
      // create upload heap
      D3D12MA::CALLOCATION_DESC vBufferUploadAllocDesc = D3D12MA::CALLOCATION_DESC{ D3D12_HEAP_TYPE_UPLOAD };
      D3D12_RESOURCE_DESC vertexBufferUploadResourceDesc = {};
      vertexBufferUploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      vertexBufferUploadResourceDesc.Alignment = 0;
      vertexBufferUploadResourceDesc.Width = vBufferSize;
      vertexBufferUploadResourceDesc.Height = 1;
      vertexBufferUploadResourceDesc.DepthOrArraySize = 1;
      vertexBufferUploadResourceDesc.MipLevels = 1;
      vertexBufferUploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
      vertexBufferUploadResourceDesc.SampleDesc.Count = 1;
      vertexBufferUploadResourceDesc.SampleDesc.Quality = 0;
      vertexBufferUploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      vertexBufferUploadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
      ComPtr<ID3D12Resource> vBufferUploadHeap;
      D3D12MA::Allocation* vBufferUploadHeapAllocation = nullptr;
      m_allocator->CreateResource(
        &vBufferUploadAllocDesc,
        &vertexBufferUploadResourceDesc, // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        &vBufferUploadHeapAllocation,
        IID_PPV_ARGS(&vBufferUploadHeap));
      vBufferUploadHeap->SetName(L"VertexBufferUploadResHeap");
      vBufferUploadHeapAllocation->SetName(L"VBUploadResHeapAlloc");

      // store vertex buffer in upload heap
      D3D12_SUBRESOURCE_DATA vertexData = {};
      vertexData.pData = reinterpret_cast<BYTE*>(triangleVertices); // pointer to our vertex array
      vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
      vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

      m_commandList->Reset(m_commandAllocator[m_frameIndex].Get(), NULL);

      // we are now creating a command with the command list to copy the data from
      // the upload heap to the default heap
      UINT64 r = UpdateSubresources(m_commandList.Get(), m_vertexDefault.Get(), vBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
      assert(r);

      // transition the vertex buffer data from copy destination state to vertex buffer state
      D3D12_RESOURCE_BARRIER vbBarrier = {};
      vbBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      vbBarrier.Transition.pResource = m_vertexDefault.Get();
      vbBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
      vbBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
      vbBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      m_commandList->ResourceBarrier(1, &vbBarrier);

      // Initialize the vertex buffer view.
      m_vertexBufferView.BufferLocation = m_vertexDefault->GetGPUVirtualAddress();
      m_vertexBufferView.StrideInBytes = sizeof(Vertex);
      m_vertexBufferView.SizeInBytes = vBufferSize;


      // Now we execute the command list to upload the initial assets (triangle data)
      m_commandList->Close();
      ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
      m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

      // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
      WaitGPUIdle(m_frameIndex);

      //textureUploadAllocation->Release();
      //iBufferUploadHeapAllocation->Release();
      vBufferUploadHeapAllocation->Release();

      //-------------------------------------------------------------ALLLOCC

      //// Note: using upload heaps to transfer static data like vert buffers is not 
      //// recommended. Every time the GPU needs it, the upload heap will be marshalled 
      //// over. Please read up on Default Heap usage. An upload heap is used here for 
      //// code simplicity and because there are very few verts to actually transfer.

      //auto heapprop1 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
      //auto resodesc1 = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

      //m_device->CreateCommittedResource(
      //  &heapprop1,
      //  D3D12_HEAP_FLAG_NONE,
      //  &resodesc1,
      //  D3D12_RESOURCE_STATE_GENERIC_READ,
      //  nullptr,
      //  IID_PPV_ARGS(&m_vertexUpload));


      //auto heapprop2 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
      //auto resodesc2 = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

      //m_device->CreateCommittedResource(
      //  &heapprop2,
      //  D3D12_HEAP_FLAG_NONE,
      //  &resodesc2,
      //  D3D12_RESOURCE_STATE_COPY_DEST,//D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
      //  nullptr,
      //  IID_PPV_ARGS(&m_vertexDefault));


      ////auto heapprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
      ////auto resodesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

      ////m_device->CreateCommittedResource(
      ////  &heapprop,
      ////  D3D12_HEAP_FLAG_NONE,
      ////  &resodesc,
      ////  D3D12_RESOURCE_STATE_GENERIC_READ,
      ////  nullptr,
      ////  IID_PPV_ARGS(&m_vertexBuffer));

      ////// Copy the triangle data to the vertex buffer.
      ////UINT8* pVertexDataBegin;
      ////CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
      ////m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
      ////memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
      ////m_vertexBuffer->Unmap(0, nullptr);


      //      // Copy the triangle data to the vertex buffer.
      //UINT8* pVertexDataBegin;
      //CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
      //m_vertexUpload->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
      //memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
      //m_vertexUpload->Unmap(0, nullptr);

      ////// Initialize the vertex buffer view.
      ////m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
      ////m_vertexBufferView.StrideInBytes = sizeof(Vertex);
      ////m_vertexBufferView.SizeInBytes = vertexBufferSize;

      //      // Initialize the vertex buffer view.
      //m_vertexBufferView.BufferLocation = m_vertexDefault->GetGPUVirtualAddress();
      //m_vertexBufferView.StrideInBytes = sizeof(Vertex);
      //m_vertexBufferView.SizeInBytes = vertexBufferSize;

    }

    //// Create synchronization objects and wait until assets have been uploaded to the GPU.
    //{
    //  m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    //  m_fenceValues[m_frameIndex]++;

    //  // Create an event handle to use for frame synchronization.
    //  m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    //  if (m_fenceEvent == nullptr) { HRESULT_FROM_WIN32(GetLastError()); }

    //  // Wait for the command list to execute; we are reusing the same command 
    //  // list in our main loop but for now, we just want to wait for setup to 
    //  // complete before continuing.
    //  WaitForGpu();
    //}


    //WaitGPUIdle(m_frameIndex);
  }

  void Graphics::DX12Render()
  {
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    WaitForFrame(m_frameIndex);
    m_fenceValues[m_frameIndex]++;

    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    m_commandQueue->Signal(m_fence[m_frameIndex].Get(), m_fenceValues[m_frameIndex]);
    // Present the frame.
    m_swapChain->Present(1, 0);

    //MoveToNextFrame();
  }

  void Graphics::DX12Destroy()
  {
    for (size_t i = 0; i < FRAME_COUNT; ++i)
    {
      WaitForFrame(i);
      m_commandQueue->Wait(m_fence[i].Get(), m_fenceValues[i]);
    }
    WaitGPUIdle(0);
    CloseHandle(m_fenceEvent);
    //WaitForGpu();
    //CloseHandle(m_fenceEvent);
  }

  void Graphics::KeyUp(uint8_t key)
  {
  }

  void Graphics::KeyDn(uint8_t key)
  {
    needUpdate = true;
  }

  //void Graphics::WaitForGpu()
  //{
  //  // la senal es DEL commandqueue NO del fence de CPU
  //  m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]);

  //  m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
  //  WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

  //  // se va a senalizar con un valor aumentado.
  //  m_fenceValues[m_frameIndex]++;
  //}

  //void Graphics::MoveToNextFrame()
  //{
  //  // Se le dice que signalize, pero lo va a hacer hasta que termine.
  //  const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
  //  m_commandQueue->Signal(m_fence.Get(), currentFenceValue);

  //  m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

  //  if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
  //  {
  //    m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
  //    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
  //  }

  //  // Se aumenta el fence pero solo del frame actualmente en le swapchain.
  //  m_fenceValues[m_frameIndex] = currentFenceValue + 1;
  //}

  void Graphics::WaitForFrame(size_t frameIndex) // wait until gpu is finished with command list
  {
    // if the current g_Fences value is still less than "g_FenceValues", then we know the GPU has not finished executing
    // the command queue since it has not reached the "g_CommandQueue->Signal(g_Fences, g_FenceValues)" command
    if (m_fence[frameIndex]->GetCompletedValue() < m_fenceValues[frameIndex])
    {
      // we have the g_Fences create an event which is signaled once the g_Fences's current value is "g_FenceValues"
      m_fence[frameIndex]->SetEventOnCompletion(m_fenceValues[frameIndex], m_fenceEvent);

      // We will wait until the g_Fences has triggered the event that it's current value has reached "g_FenceValues". once it's value
      // has reached "g_FenceValues", we know the command queue has finished executing
      WaitForSingleObject(m_fenceEvent, INFINITE);
    }
  }

  void Graphics::WaitGPUIdle(size_t frameIndex)
  {
    m_fenceValues[frameIndex]++;
    m_commandQueue->Signal(m_fence[frameIndex].Get(), m_fenceValues[frameIndex]);
    WaitForFrame(frameIndex);
  }

  void Graphics::PopulateCommandList()
  {
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    m_commandAllocator[m_frameIndex]->Reset();
    m_commandList->Reset(m_commandAllocator[m_frameIndex].Get(), m_pipelineState.Get());

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    auto resRT = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(),
      D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    m_commandList->ResourceBarrier(1, &resRT);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
      m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //m_commandList->CopyResource(m_vertexDefault.Get(), m_vertexUpload.Get());
    //auto resVBB = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexDefault.Get(),
    //  D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    //m_commandList->ResourceBarrier(1, &resVBB);

    //if (needRefresh) {
    //  PreFillCL();
    //  needRefresh = false;
    //}

    //if (needUpdate) {
    //  PostFillCL();
    //  needUpdate = false;
    //}

    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->DrawInstanced(3, 1, 0, 0);

    //auto resDFF = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexDefault.Get(),
    //  D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);

    //m_commandList->ResourceBarrier(1, &resDFF);

    // Indicate that the back buffer will now be used to present.
    auto resState = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(),
      D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &resState);

    m_commandList->Close();
  }

  void Graphics::PreFillCL()
  {
    //auto resDFF = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexDefault.Get(),
    //  D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    //m_commandList->ResourceBarrier(1, &resDFF);

    m_commandList->CopyResource(m_vertexDefault.Get(), m_vertexUpload.Get());

    auto resVBB = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexDefault.Get(),
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    m_commandList->ResourceBarrier(1, &resVBB);

    //m_vertexBufferView.BufferLocation = m_vertexDefault->GetGPUVirtualAddress();

  }

  void Graphics::PostFillCL()
  {
    auto resDFF = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexDefault.Get(),
    D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    m_commandList->ResourceBarrier(1, &resDFF);

    m_commandList->CopyResource(m_vertexDefault.Get(), m_vertexUpload.Get());

    auto resVBB = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexDefault.Get(),
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    m_commandList->ResourceBarrier(1, &resVBB);
  }

}