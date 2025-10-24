#pragma once

class PixelCornersBlock;

class PixelCornersMenu
{
  public:
    PixelCornersMenu() = delete;
    explicit PixelCornersMenu(PixelCornersBlock* pixel_block);
    ~PixelCornersMenu() = default;

    void RenderMenu();

private:
    PixelCornersBlock* pixelBlock;
    float sigmaFactor = 1.0f;
    float kFactor = 0.04f;
};
