#include "GameMain.h"
#include "..\helper\allheader.h"

namespace JUCore
{
  bool gIsSupending = false;
  static std::shared_ptr<IGameApp> g_App = nullptr;

  static HINSTANCE g_Instance;
  static HWND g_Hwnd;

  void InitApp()
  {
    g_App->Startup(g_Hwnd);
  }

  void EndApp()//IGameApp& game)
  {
    g_App->Cleanup();
    //Graphics::get().DX12Destroy();

    //GameInput::Shutdown();
  }

  bool UpdateApp()
  {

    //float DeltaTime = Graphics::GetFrameTime();

    g_App->Update(0.f);// DeltaTime);
    g_App->RenderScene();

    return !g_App->IsDone();
  }

  // Default implementation to be overridden by the application
  bool IGameApp::IsDone(void)
  {
    return false;// GameInput::IsFirstPressed(GameInput::kKey_escape);
  }

  LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    switch (message)
    {
    case WM_SIZE:
      return 0;

    case WM_KEYDOWN:
        g_App->OnKeyDown(static_cast<uint8_t>(wParam));
      return 0;

    case WM_KEYUP:
        g_App->OnKeyUp(static_cast<uint8_t>(wParam));
      return 0;

    case WM_PAINT:
        UpdateApp();
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
    g_App = app;

    g_Instance = (HINSTANCE)GetModuleHandle(NULL);
    CoInitialize(NULL);

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

    InitApp();

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }

    EndApp();
    //Graphics::Shutdown();
    return (int)msg.wParam;
  }

}