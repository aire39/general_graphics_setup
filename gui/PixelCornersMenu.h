#pragma once

#include <array>

class PixelCornersBlock;
class NonMaximumSuppressBlock;

class PixelCornersMenu
{
  public:
    PixelCornersMenu() = delete;
    explicit PixelCornersMenu(PixelCornersBlock* pixel_block, NonMaximumSuppressBlock* nms_block);
    ~PixelCornersMenu() = default;

    void RenderMenu();

private:
    PixelCornersBlock* pixelBlock;
    NonMaximumSuppressBlock* nmsBlock;
    float sigmaFactor = 1.0f;
    float kFactor = 0.04f;
    float responseFactor = 0.1f;
    float minDistanceFactor = 10.0f;
    float pointSize = 4.0f;
    std::array<float, 3> pointColor = {0.384f, 0.553f, 0.867f};
};
