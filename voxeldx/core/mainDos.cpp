#include "GameMain.h"
#include "..\helper\allheader.h"
//#include <thread>
//#include <chrono>

using namespace JUCore;

static void PrintLogo()
{
  PRINT(WINDOW_TITLE);
}

int maindos(int argc, wchar_t** argv)
{
  PrintLogo();
  //GameMain mEngino(900, 675);
  auto mEngino = std::make_shared<GameMain>(900,675);

  //std::this_thread::sleep_for(std::chrono::milliseconds(50));

  return JUCore::RunApplication(mEngino);// , L"MiniEngino", hInstance, nCmdShow);
}


//int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int nCmdShow)
int wmain(int argc, wchar_t** argv)
{
  try
  {
    return maindos(argc, argv);
  }
  CATCH_PRINT_ERROR(return (int)ExitCode::RuntimeError;)
}