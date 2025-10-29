#pragma once

#include <cstdint>
#include <array>

class PixelCornersBlock;
class NonMaximumSuppressBlock;
class OpticalFlowBlock;

class PixelCornersMenu
{
  public:
    PixelCornersMenu() = delete;
    explicit PixelCornersMenu(PixelCornersBlock* pixel_block, NonMaximumSuppressBlock* nms_block, OpticalFlowBlock* optical_block);
    ~PixelCornersMenu() = default;

    void RenderMenu();

  private:
    enum class CornerSettings {CORNERS, FLOW};

    PixelCornersBlock* pixelBlock;
    NonMaximumSuppressBlock* nmsBlock;
    OpticalFlowBlock* opticalBlock;
    CornerSettings settings = CornerSettings::CORNERS;
    float sigmaFactor = 1.0f;
    float kFactor = 0.04f;
    float responseFactor = 0.1f;
    float minDistanceFactor = 10.0f;
    float pointSize = 4.0f;
    int32_t flowSteps = 50;
    int32_t minFlowAmount = 50;
    std::array<float, 3> pointColor = {0.384f, 0.553f, 0.867f};
};
