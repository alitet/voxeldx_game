#pragma once

#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOHELP
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace JUCore
{
  //extern bool gIsSupending;

  class IGameApp
  {
  public:
    virtual void Startup(void) = 0;
    virtual void Cleanup(void) = 0;
   
    virtual bool IsDone(void);

    virtual void Update(float deltaT) = 0;

    virtual void RenderScene(void) = 0;

    virtual void RenderUI(class GraphicsContext&) {};

    virtual bool RequiresRaytracingSupport() const { return false; }
  };
}

namespace JUCore
{
  int RunApplication(IGameApp& app, const wchar_t* className, HINSTANCE hInst, int nCmdShow);
}

//#define CREATE_APPLICATION( app_class ) \
//    int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int nCmdShow) \
//    { \
//        return JUCore::RunApplication( app_class(), L#app_class, hInstance, nCmdShow ); \
//    }