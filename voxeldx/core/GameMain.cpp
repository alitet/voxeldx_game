#include "GameMain.h"
//#include <shellapi.h>
//#include "../graphics/Graphics.h"

//#pragma comment(lib, "runtimeobject.lib") 

//namespace JUCore
//{
  //CREATE_APPLICATION(MiniEngino)

  void GameMain::Startup(HWND wndh)
  {
    auto dimos = GetDims();
    mGraphics.DX12Initialize(dimos.first, dimos.second, wndh);
    mGraphics.DX12ConfigLoad();
  }

  void GameMain::Cleanup(void)
  {
    mGraphics.DX12Destroy();
  }

  void GameMain::Update(float /*deltaT*/)
  {
    //ScopedTimer _prof(L"Update State");

    // Update something
  }

  void GameMain::RenderScene(void)
  {
    //GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

    //gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    //gfxContext.ClearColor(g_SceneColorBuffer);
    //gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
    //gfxContext.SetViewportAndScissor(0, 0, g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());

    //// Rendering something

    mGraphics.DX12Render();

    //gfxContext.Finish();
  }

  void GameMain::OnKeyUp(uint8_t key)
  {
  }

  void GameMain::OnKeyDown(uint8_t key)
  {
    mGraphics.KeyDn(key);
  }

  std::pair<unsigned int, unsigned int> GameMain::GetDims()
  {
    return std::make_pair(mWidth, mHeight);
  }

//}