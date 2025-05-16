#include "GameMain.h"
#include <thread>
#include <chrono>

using namespace JUCore;

//class MiniEngino : public JUCore::IGameApp
//{
//public:
//
//  MiniEngino(unsigned int width, unsigned int height)
//    : mWidth(width), mHeight(height)
//  {
//  }
//
//  void Startup(void) override;
//  void Cleanup(void) override;
//
//  void Update(float deltaT) override;
//  void RenderScene(void) override;
//
//  void OnKeyUp(uint8_t) override;
//  void OnKeyDown(uint8_t) override;
//
//  std::pair<unsigned int, unsigned int> GetDims() override;
//
//private:
//  unsigned int mWidth, mHeight;
//
//};
//
////CREATE_APPLICATION(MiniEngino)
//
//void MiniEngino::Startup(void)
//{
//  // Setup your data
//}
//
//void MiniEngino::Cleanup(void)
//{
//  // Free up resources in an orderly fashion
//}
//
//void MiniEngino::Update(float /*deltaT*/)
//{
//  //ScopedTimer _prof(L"Update State");
//
//  // Update something
//}
//
//void MiniEngino::RenderScene(void)
//{
//	//GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");
//
//	//gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
//	//gfxContext.ClearColor(g_SceneColorBuffer);
//	//gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
//	//gfxContext.SetViewportAndScissor(0, 0, g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());
//
//	//// Rendering something
//
//	//gfxContext.Finish();
//}
//
//void MiniEngino::OnKeyUp(uint8_t key)
//{
//}
//
//void MiniEngino::OnKeyDown(uint8_t key)
//{
//}
//
//std::pair<unsigned int, unsigned int> MiniEngino::GetDims()
//{
//  return std::make_pair(mWidth, mHeight);
//}


//MiniEngino mEngino;

//int main()
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int nCmdShow)
{
  GameMain mEngino(900, 675);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  return JUCore::RunApplication(mEngino, L"MiniEngino", hInstance, nCmdShow);
}