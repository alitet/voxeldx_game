#include "Graphics.h"

//#include <d3d12.h>
//#include "include/directx/d3dx12.h"

//#include <dxgi1_6.h>
//#include <D3Dcompiler.h>
//#include <DirectXMath.h>
//#include "include/directx/d3dx12/d3dx12.h"

//#include <shellapi.h>

//ComPtr<ID3D12Device> JUCore::Graphics::m_device = nullptr;
//ComPtr<ID3D12CommandQueue> JUCore::Graphics::m_commandQueue = nullptr;
//ComPtr<IDXGISwapChain3> JUCore::Graphics::m_swapChain = nullptr;
//ComPtr<ID3D12DescriptorHeap> JUCore::Graphics::m_rtvHeap = nullptr;
//ComPtr<ID3D12Resource> JUCore::Graphics::m_renderTargets[] = { nullptr };
//ComPtr<ID3D12CommandAllocator> JUCore::Graphics::m_commandAllocator = nullptr;

std::unique_ptr<JUCore::Graphics> JUCore::Graphics::mInstance(new JUCore::Graphics);

//const UINT JUCore::Graphics::frameCount = 2;

//UINT JUCore::Graphics::frameIndex = 0;
//UINT JUCore::Graphics::m_rtvDescriptorSize = 0;

namespace JUCore
{
  std::wstring GetAssetFullPath(LPCWSTR assetName)
  {
    return L"data/shaders/" + std::wstring(assetName);
  }

  Graphics::Graphics()
    : m_frameIndex(0), m_rtvDescriptorSize(0)
    , m_aspectRatio(1.f)
  {
  }

  Graphics& Graphics::get()
  {
    if (!mInstance) {
      mInstance = std::unique_ptr<Graphics>(new Graphics());
    }
    return *mInstance;
  }

  // from Microsoft MiniEngime
  _Use_decl_annotations_ void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory,
    _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
    bool requestHighPerformanceAdapter = false)
  {
    *ppAdapter = nullptr;
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

    //if (adapter.Get() == nullptr) {
    //  for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex) {
    //    DXGI_ADAPTER_DESC1 desc;
    //    adapter->GetDesc1(&desc);

    //    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) { continue; }
    //    if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
    //      break;
    //    }
    //  }
    //}
    *ppAdapter = adapter.Detach();
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

    //if (m_useWarpDevice)
    //{
    //  ComPtr<IDXGIAdapter> warpAdapter;
    //  ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

    //  ThrowIfFailed(D3D12CreateDevice(
    //    warpAdapter.Get(),
    //    D3D_FEATURE_LEVEL_11_0,
    //    IID_PPV_ARGS(&m_device)
    //  ));
    //}
    //else
    //{
    ComPtr<IDXGIAdapter1> hardwareAdapter;
    GetHardwareAdapter(factory.Get(), &hardwareAdapter);

    D3D12CreateDevice(hardwareAdapter.Get(),
      D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
    //}

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = frameCount;
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

    // Create descriptor heaps.
    {
      // Describe and create a render target view (RTV) descriptor heap.
      D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
      rtvHeapDesc.NumDescriptors = frameCount;
      rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));

      m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
      CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

      // Create a RTV for each frame.
      for (UINT n = 0; n < frameCount; n++)
      {
        m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
        m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
      }
    }

    m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
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

    // Create the pipeline state, which includes compiling and loading shaders.
    {
      ComPtr<ID3DBlob> vertexShader;
      ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
      // Enable better shader debugging with the graphics debugging tools.
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

    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList));
    // cerrado porque no graba nada
    m_commandList->Close();

    // Create the vertex buffer.
    {
      // Define the geometry for a triangle.
      Vertex triangleVertices[] =
      {
          { { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
          { { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
          { { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
      };

      const UINT vertexBufferSize = sizeof(triangleVertices);

      // Note: using upload heaps to transfer static data like vert buffers is not 
      // recommended. Every time the GPU needs it, the upload heap will be marshalled 
      // over. Please read up on Default Heap usage. An upload heap is used here for 
      // code simplicity and because there are very few verts to actually transfer.
      auto heapprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
      auto resodesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

      m_device->CreateCommittedResource(
        &heapprop,
        D3D12_HEAP_FLAG_NONE,
        &resodesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer));

      // Copy the triangle data to the vertex buffer.
      UINT8* pVertexDataBegin;
      CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
      m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
      memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
      m_vertexBuffer->Unmap(0, nullptr);

      // Initialize the vertex buffer view.
      m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
      m_vertexBufferView.StrideInBytes = sizeof(Vertex);
      m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
      m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
      m_fenceValues[m_frameIndex]++;

      // Create an event handle to use for frame synchronization.
      m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
      if (m_fenceEvent == nullptr) { HRESULT_FROM_WIN32(GetLastError()); }

      // Wait for the command list to execute; we are reusing the same command 
      // list in our main loop but for now, we just want to wait for setup to 
      // complete before continuing.
      WaitForGpu();
    }
  }

  void Graphics::DX12Render()
  {
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    m_swapChain->Present(1, 0);

    MoveToNextFrame();
  }

  void Graphics::DX12Destroy()
  {
    WaitForGpu();
    CloseHandle(m_fenceEvent);
  }

  void Graphics::WaitForGpu()
  {
    // la senal es DEL commandqueue NO del fence de CPU
    m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]);

    m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
    WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

    // se va a senalizar con un valor aumentado.
    m_fenceValues[m_frameIndex]++;
  }

  void Graphics::MoveToNextFrame()
  {
    // Se le dice que signalize, pero lo va a hacer hasta que termine.
    const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
    m_commandQueue->Signal(m_fence.Get(), currentFenceValue);

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
    {
      m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
      WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
    }

    // Se aumenta el fence pero solo del frame actualmente en le swapchain.
    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
  }

  void Graphics::PopulateCommandList()
  {
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    m_commandAllocator->Reset();
    m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());

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
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->DrawInstanced(3, 1, 0, 0);

    // Indicate that the back buffer will now be used to present.
    auto resState = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(),
      D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &resState);

    m_commandList->Close();
  }

}