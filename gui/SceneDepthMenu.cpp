#include "SceneDepthMenu.h"

#include <string>
#include <algorithm>
#include <imgui.h>

#include "process/SceneDepthBlock.h"

namespace {
  constexpr float default_color_wheel_size = 150.0f;

  [[maybe_unused]] void SetItemWidth(const float item_width)
  {
    // Get the available content width
    const float content_width = ImGui::GetContentRegionAvail().x;

    // Calculate the horizontal offset to center the color picker
    const float offset = (content_width - item_width) * 0.5f;

    // Move the cursor to the new centered position
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

    // Set the item width and draw the color picker
    ImGui::SetNextItemWidth(item_width);
  }

  void TextCentered(const std::string& text)
  {
    // Get the current window's available width
    const float windowWidth = ImGui::GetWindowSize().x;

    // Calculate the width of the text
    const float textWidth = ImGui::CalcTextSize(text.c_str()).x;

    // Calculate the X position to center the text
    const float xPos = (windowWidth - textWidth) * 0.5f;

    // Set the cursor position to this calculated X position
    ImGui::SetCursorPosX(xPos);

    // Render the text
    ImGui::Text("%s", text.c_str());
  }

  [[maybe_unused]] std::array<double, 3> ConvertColorFloatToDouble(std::array<float, 3> color)
  {
    std::array<double, 3> convert_to_double;
    std::ranges::transform(color, convert_to_double.begin(), [](const float& v) { return static_cast<double>(v); });
    return convert_to_double;
  }
}

SceneDepthMenu::SceneDepthMenu(SceneDepthBlock* scene_depth_block)
  : sceneDepthBlock(scene_depth_block)
{
}

void SceneDepthMenu::RenderMenu()
{
  ImGui::Begin("Scene Depth Control", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  if (sceneDepthBlock)
  {
    ImGui::PushID(1);
    ImGui::BeginGroup();

    TextCentered("Ref Translate");
    if (ImGui::SliderFloat("##RSTranslateX", &refTranslate[0], -100.0f, 100.0f, "%.2f")) sceneDepthBlock->SetCameraRefPosition(refTranslate);
    ImGui::SameLine();
    if (ImGui::InputFloat("##RITranslateX", &refTranslate[0], 0.0f, 1.0f, "%.2f")) sceneDepthBlock->SetCameraRefPosition(refTranslate);

    if (ImGui::SliderFloat("##RSTranslateY", &refTranslate[1], -100.0f, 100.0f, "%.2f")) sceneDepthBlock->SetCameraRefPosition(refTranslate);
    ImGui::SameLine();
    if (ImGui::InputFloat("##RITranslateY", &refTranslate[1], 0.0f, 1.0f, "%.2f")) sceneDepthBlock->SetCameraRefPosition(refTranslate);

    if (ImGui::SliderFloat("##RSTranslateZ", &refTranslate[2], -100.0f, 100.0f, "%.2f")) sceneDepthBlock->SetCameraRefPosition(refTranslate);
    ImGui::SameLine();
    if (ImGui::InputFloat("##RITranslateZ", &refTranslate[2], 0.0f, 1.0f, "%.2f")) sceneDepthBlock->SetCameraRefPosition(refTranslate);

    TextCentered("Ref Focal Factor");
    if (ImGui::SliderFloat("##RSFocal0", &refFocal[0], -100.0, 100.0f, "%.3f")) sceneDepthBlock->SetCameraRefFocal(refFocal);
    ImGui::SameLine();
    if (ImGui::InputFloat("##RIFocal0", &refFocal[0], 0.0f, 1.0f, "%.2f")) sceneDepthBlock->SetCameraRefFocal(refFocal);

    if (ImGui::SliderFloat("##RSFocal1", &refFocal[1], -100.0, 100.0f, "%.3f")) sceneDepthBlock->SetCameraRefFocal(refFocal);
    ImGui::SameLine();
    if (ImGui::InputFloat("##RIFocal1", &refFocal[1], 0.0f, 1.0f, "%.2f")) sceneDepthBlock->SetCameraRefFocal(refFocal);

    // ==========================================================================================================================================================================

    TextCentered("Cam Translate");
    if (ImGui::SliderFloat("##STranslateX", &camTranslate[0], -100.0f, 100.0f, "%.2f")) sceneDepthBlock->SetCameraPosition(camTranslate, 0);
    ImGui::SameLine();
    if (ImGui::InputFloat("##iTranslateX", &camTranslate[0], 0.0f, 1.0f, "%.2f")) sceneDepthBlock->SetCameraPosition(camTranslate, 0);

    if (ImGui::SliderFloat("##STranslatey", &camTranslate[1], -100.0f, 100.0f, "%.2f")) sceneDepthBlock->SetCameraPosition(camTranslate, 0);
    ImGui::SameLine();
    if (ImGui::InputFloat("##iTranslatey", &camTranslate[1], 0.0f, 1.0f, "%.2f")) sceneDepthBlock->SetCameraPosition(camTranslate, 0);

    if (ImGui::SliderFloat("##STranslateZ", &camTranslate[2], -100.0f, 100.0f, "%.2f")) sceneDepthBlock->SetCameraPosition(camTranslate, 0);
    ImGui::SameLine();
    if (ImGui::InputFloat("##iTranslateZ", &camTranslate[2], 0.0f, 1.0f, "%.2f")) sceneDepthBlock->SetCameraPosition(camTranslate, 0);

    TextCentered("Cam Focal Factor");
    if (ImGui::SliderFloat("##SFocal0", &camFocal[0], -100.0, 100.0f, "%.3f")) sceneDepthBlock->SetCameraRefFocal(camFocal);
    ImGui::SameLine();
    if (ImGui::InputFloat("##IFocal0", &camFocal[0], 0.0f, 1.0f, "%.2f")) sceneDepthBlock->SetCameraRefFocal(camFocal);

    if (ImGui::SliderFloat("##SFocal1", &camFocal[1], -100.0, 100.0f, "%.3f")) sceneDepthBlock->SetCameraRefFocal(camFocal);
    ImGui::SameLine();
    if (ImGui::InputFloat("##IFocal1", &camFocal[1], 0.0f, 1.0f, "%.2f")) sceneDepthBlock->SetCameraRefFocal(camFocal);

/*
    TextCentered("Cam Translate");
    if (ImGui::SliderFloat("##RTranslate", &camTranslate[0], 0.0, 1.0f, "%.3f")) sceneDepthBlock->SetCameraPosition(camTranslate, 0);
    TextCentered("Cam Focal");
    if (ImGui::SliderFloat("##RTranslate", &camFocal[0], 0.0, 100.0f, "%.3f")) sceneDepthBlock->SetCameraFocal(camFocal, 0);
*/
    ImGui::EndGroup();
    ImGui::PopID();

    ImGui::End();
  }
}

