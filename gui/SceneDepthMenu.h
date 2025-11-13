#pragma once

#include <cstdint>
#include <array>

class SceneDepthBlock;

class SceneDepthMenu
{
  public:
    SceneDepthMenu() = delete;
    explicit SceneDepthMenu(SceneDepthBlock* scene_depth_block);
    ~SceneDepthMenu() = default;

    void RenderMenu();

  private:
    enum class CornerSettings {CORNERS, FLOW};

    SceneDepthBlock* sceneDepthBlock;
    std::array<float, 3> refTranslate = {-5.0f, 0.0f, 0.0f};
    std::array<float, 2> refFocal = {1.0f, 1.0f};

    std::array<float, 3> camTranslate = {0.0f, 1.05f, 0.0f};
    std::array<float, 2> camFocal = {1.0f, 1.0f};
};
