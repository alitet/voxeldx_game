#include "GameMain.h"
#include "..\helper\allheader.h"
//#include <memory>
//#include <shellapi.h>
//#include "../graphics/Graphics.h"

//#pragma comment(lib, "runtimeobject.lib") 

namespace JUCore
{

  //class DXGIUsage
  //{
  //public:
  //  void Init();
  //  IDXGIFactory4* GetDXGIFactory() const { return m_DXGIFactory.Get(); }
  //  void PrintAdapterList() const;
  //  // If failed, returns null pointer.
  //  ComPtr<IDXGIAdapter1> CreateAdapter(const GPUSelection& GPUSelection) const;

  //private:
  //  ComPtr<IDXGIFactory4> m_DXGIFactory;
  //};


 // using namespace Graphics;

  bool gIsSupending = false;
  //static HWND g_hWnd = nullptr;
  static std::shared_ptr<IGameApp> g_App = nullptr;

  static HINSTANCE g_Instance;
  static HWND g_Hwnd;

  void InitializeApplication()// IGameApp& game, HWND wndh)
  {
    //int argc = 0;
    //LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    //CommandLineArgs::Initialize(argc, argv);

    //Graphics::Initialize(game.RequiresRaytracingSupport());
    //SystemTime::Initialize();
    //GameInput::Initialize();
    //EngineTuning::Initialize();
    //auto dimos = game.GetDims();
    g_App->Startup(g_Hwnd);

    //Graphics::get().DX12Initialize(dimos.first, dimos.second, g_hWnd);
    //Graphics::get().DX12ConfigLoad();

    //game.Startup();
  }

  void TerminateApplication()//IGameApp& game)
  {
    //g_CommandManager.IdleGPU();

    g_App->Cleanup();
    //Graphics::get().DX12Destroy();

    //GameInput::Shutdown();
  }

  bool UpdateApplication()//IGameApp& game)
  {
    //EngineProfiling::Update();

    //float DeltaTime = Graphics::GetFrameTime();

    //GameInput::Update(DeltaTime);
    //EngineTuning::Update(DeltaTime);

    g_App->Update(0.f);// DeltaTime);
    g_App->RenderScene();

    //Graphics::get().DX12Render();

    //PostEffects::Render();

    //GraphicsContext& UiContext = GraphicsContext::Begin(L"Render UI");
    //UiContext.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    //UiContext.ClearColor(g_OverlayBuffer);
    //UiContext.SetRenderTarget(g_OverlayBuffer.GetRTV());
    //UiContext.SetViewportAndScissor(0, 0, g_OverlayBuffer.GetWidth(), g_OverlayBuffer.GetHeight());
    //game.RenderUI(UiContext);

    //UiContext.SetRenderTarget(g_OverlayBuffer.GetRTV());
    //UiContext.SetViewportAndScissor(0, 0, g_OverlayBuffer.GetWidth(), g_OverlayBuffer.GetHeight());
    //EngineTuning::Display(UiContext, 10.0f, 40.0f, 1900.0f, 1040.0f);

    //UiContext.Finish();

    //Display::Present();

    return !g_App->IsDone();
  }

  // Default implementation to be overridden by the application
  bool IGameApp::IsDone(void)
  {
    return false;// GameInput::IsFirstPressed(GameInput::kKey_escape);
  }

  LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    //auto kk = GetWindowLongPtr(hWnd, GWLP_USERDATA);
    //IGameApp* pApp = reinterpret_cast<IGameApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    //case WM_CREATE:
    //{
    //  // Save the DXSample* passed in to CreateWindow.
    //  LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
    //  SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    //}
    //return 0;

    case WM_SIZE:
      return 0;

    case WM_KEYDOWN:
      if (g_App != nullptr) {
        uint8_t kcd = static_cast<uint8_t>(wParam);
        g_App->OnKeyDown(kcd);
        //Graphics::get().KeyDn(kcd);
      }
      return 0;

    case WM_KEYUP:
      if (g_App != nullptr) {
        g_App->OnKeyUp(static_cast<uint8_t>(wParam));
      }
      return 0;

    case WM_PAINT:
      //if (g_App != nullptr) {
        UpdateApplication();
      //}
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
  }

  int RunApplication(std::shared_ptr<IGameApp> app)//, const wchar_t* className, HINSTANCE hInst, int nCmdShow)
  {
    g_App = app;// std::unique_ptr<IGameApp>(&app);

    //gapp = &app;
    //if (!XMVerifyCPUSupport())
    //  return 1;

    //Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);
    //ASSERT_SUCCEEDED(InitializeWinRT);

    g_Instance = (HINSTANCE)GetModuleHandle(NULL);
    CoInitialize(NULL);

    //// Register class
    //WNDCLASSEX wcex;
    //wcex.cbSize = sizeof(WNDCLASSEX);
    //wcex.style = CS_HREDRAW | CS_VREDRAW;
    //wcex.lpfnWndProc = WndProc;
    //wcex.cbClsExtra = 0;
    //wcex.cbWndExtra = 0;
    //wcex.hInstance = g_Instance;// hInst;
    //wcex.hIcon = LoadIcon(g_Instance, IDI_APPLICATION);//LoadIcon(hInst, IDI_APPLICATION);
    //wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    //wcex.lpszMenuName = nullptr;
    //wcex.lpszClassName = CLASS_NAME;
    //wcex.hIconSm = LoadIcon(g_Instance, IDI_APPLICATION); //LoadIcon(hInst, IDI_APPLICATION);
    ////ASSERT(0 != RegisterClassEx(&wcex), "Unable to register a window");
    //auto hh = RegisterClassEx(&wcex);

    //// Create window
    //auto dimos = app.GetDims();
    //RECT rc = { 0, 0, (LONG)dimos.first, (LONG)dimos.second };
    //AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    ////IGameApp* japp = &app;

    ////HWND g_hWnd = CreateWindow(className, className, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
    ////  rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, &app);

    //g_Hwnd = CreateWindow(CLASS_NAME, WINDOW_TITLE, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
    //  rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, g_Instance, &app);

    ////ASSERT(g_hWnd != 0);

    ////InitializeApplication(app, g_hWnd);

    ////ShowWindow(g_hWnd, nCmdShow/*SW_SHOWDEFAULT*/);

    WNDCLASSEX wndClass;
    ZeroMemory(&wndClass, sizeof(wndClass));
    wndClass.cbSize = sizeof(wndClass);
    wndClass.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    wndClass.hbrBackground = NULL;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hInstance = g_Instance;
    wndClass.lpfnWndProc = &WndProc;
    wndClass.lpszClassName = CLASS_NAME;

    ATOM classR = RegisterClassEx(&wndClass);
    assert(classR);

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
    DWORD exStyle = 0;

    auto dimos = app->GetDims();
    RECT rect = { 0, 0, (LONG)dimos.first, (LONG)dimos.second };
    //RECT rect = { 0, 0, SIZE_X, SIZE_Y };
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);
    g_Hwnd = CreateWindowEx(
      exStyle,
      CLASS_NAME,
      WINDOW_TITLE,
      style,
      CW_USEDEFAULT, CW_USEDEFAULT,
      rect.right - rect.left, rect.bottom - rect.top,
      NULL,
      NULL,
      g_Instance,
      0);
    assert(g_Hwnd);

    InitializeApplication();// app, g_Hwnd);

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
      // Process any messages in the queue.
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }

    TerminateApplication();// app);
    //Graphics::Shutdown();
    return (int)msg.wParam;
  }

}