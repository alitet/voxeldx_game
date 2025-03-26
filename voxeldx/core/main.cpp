#include "GameMain.h"

using namespace JUCore;

class MiniEngino : public JUCore::IGameApp
{
public:

  MiniEngino()
  {
  }

  virtual void Startup(void) override;
  virtual void Cleanup(void) override;

  virtual void Update(float deltaT) override;
  virtual void RenderScene(void) override;

private:
};

//CREATE_APPLICATION(MiniEngino)

void MiniEngino::Startup(void)
{
  // Setup your data
}

void MiniEngino::Cleanup(void)
{
  // Free up resources in an orderly fashion
}

void MiniEngino::Update(float /*deltaT*/)
{
  //ScopedTimer _prof(L"Update State");

  // Update something
}

void MiniEngino::RenderScene(void)
{
	//GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

	//gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	//gfxContext.ClearColor(g_SceneColorBuffer);
	//gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
	//gfxContext.SetViewportAndScissor(0, 0, g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());

	//// Rendering something

	//gfxContext.Finish();
}

MiniEngino mEngino;

//int main()
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int nCmdShow)
{
  return JUCore::RunApplication(mEngino, L"MiniEngino", hInstance, nCmdShow);
}