#include "GameMain.h"
//#include <shellapi.h>
#include "../graphics/Graphics.h"

//#pragma comment(lib, "runtimeobject.lib") 

namespace JUCore
{
 // using namespace Graphics;

  bool gIsSupending = false;
  HWND g_hWnd = nullptr;

  void InitializeApplication(IGameApp& game)
  {
    //int argc = 0;
    //LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    //CommandLineArgs::Initialize(argc, argv);

    //Graphics::Initialize(game.RequiresRaytracingSupport());
    //SystemTime::Initialize();
    //GameInput::Initialize();
    //EngineTuning::Initialize();
    auto dimos = game.GetDims();
    Graphics::get().DX12Initialize(dimos.first, dimos.second, g_hWnd);
    Graphics::get().DX12ConfigLoad();

    game.Startup();
  }

  void TerminateApplication(IGameApp& game)
  {
    //g_CommandManager.IdleGPU();

    game.Cleanup();
    Graphics::get().DX12Destroy();

    //GameInput::Shutdown();
  }

  bool UpdateApplication(IGameApp& game)
  {
    //EngineProfiling::Update();

    //float DeltaTime = Graphics::GetFrameTime();

    //GameInput::Update(DeltaTime);
    //EngineTuning::Update(DeltaTime);

    game.Update(0.f);// DeltaTime);
    game.RenderScene();

    Graphics::get().DX12Render();

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

    return !game.IsDone();
  }

  // Default implementation to be overridden by the application
  bool IGameApp::IsDone(void)
  {
    return false;// GameInput::IsFirstPressed(GameInput::kKey_escape);
  }

  

  LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

  int RunApplication(IGameApp& app, const wchar_t* className, HINSTANCE hInst, int nCmdShow)
  {
    //if (!XMVerifyCPUSupport())
    //  return 1;

    //Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);
    //ASSERT_SUCCEEDED(InitializeWinRT);

    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = className;
    wcex.hIconSm = LoadIcon(hInst, IDI_APPLICATION);
    //ASSERT(0 != RegisterClassEx(&wcex), "Unable to register a window");
    auto hh = RegisterClassEx(&wcex);

    // Create window
    auto dimos = app.GetDims();
    RECT rc = { 0, 0, (LONG)dimos.first, (LONG)dimos.second };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    g_hWnd = CreateWindow(className, className, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
      rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);

    //ASSERT(g_hWnd != 0);

    InitializeApplication(app);

    ShowWindow(g_hWnd, nCmdShow/*SW_SHOWDEFAULT*/);

    do
    {
      MSG msg = {};
      bool done = false;
      while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT)
          done = true;
      }

      if (done)
        break;
    } while (UpdateApplication(app));	// Returns false to quit loop

    TerminateApplication(app);
    //Graphics::Shutdown();
    return 0;
  }

  //--------------------------------------------------------------------------------------
  // Called every time the application receives a message
  //--------------------------------------------------------------------------------------
  LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    switch (message)
    {
    case WM_SIZE:
      //Display::Resize((UINT)(UINT64)lParam & 0xFFFF, (UINT)(UINT64)lParam >> 16);
      break;

    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
  }

}