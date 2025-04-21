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

  // from Microsoft MiniEngime
  _Use_decl_annotations_ void GetHardwareAdapter(
    _In_ IDXGIFactory1* pFactory,
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
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
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

      D3D12CreateDevice( hardwareAdapter.Get(),
        D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device) );
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
    frameIndex = m_swapChain->GetCurrentBackBufferIndex();

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
    {
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

      ComPtr<ID3DBlob> signature;
      ComPtr<ID3DBlob> error;
      D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
      m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
    }
  }

}