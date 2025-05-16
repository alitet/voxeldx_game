#pragma once
#include "IGameApp.h"
#include "../graphics/Graphics.h"

class GameMain : public JUCore::IGameApp
{
public:

  GameMain(unsigned int width, unsigned int height)
    : mWidth(width), mHeight(height)
  {
  }

  void Startup(HWND) override;
  void Cleanup(void) override;

  void Update(float deltaT) override;
  void RenderScene(void) override;

  void OnKeyUp(uint8_t) override;
  void OnKeyDown(uint8_t) override;

  std::pair<unsigned int, unsigned int> GetDims() override;

private:
  unsigned int mWidth, mHeight;

  JUCore::Graphics mGraphics;
};


