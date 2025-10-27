#include "PixelCornersMenu.h"

#include <string>
#include <imgui.h>

#include "process/NonMaximumSuppressBlock.h"
#include "process/PixelCornersBlock.h"

namespace {
  constexpr float default_color_wheel_size = 150.0f;

  void SetItemWidth(const float item_width)
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
}

PixelCornersMenu::PixelCornersMenu(PixelCornersBlock* pixel_block, NonMaximumSuppressBlock* nms_block)
  : pixelBlock(pixel_block)
  , nmsBlock(nms_block)
{
  if (nmsBlock)
  {
    nmsBlock->SetPointColor(pointColor);
  }
}

void PixelCornersMenu::RenderMenu()
{
  ImGui::Begin("Pixel Corners", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  if (pixelBlock)
  {
    ImGui::PushID(1);
    ImGui::BeginGroup();

    TextCentered("K Factor");
    if (ImGui::SliderFloat("##KFactor", &kFactor, 0.01f, 0.1f, "%.2f")) pixelBlock->SetKValue(kFactor);
    TextCentered("Sigma Factor");
    if (ImGui::SliderFloat("##SigmaFactor", &sigmaFactor, 0.0, 1.0f, "%.3f")) pixelBlock->SetSigma(sigmaFactor);
    TextCentered("Response Factor");
    if (ImGui::SliderFloat("##ResponseFactor", &responseFactor, 0.0, 1.0f, "%.3f")) nmsBlock->SetResponseFactor(responseFactor);
    TextCentered("Min Distance Factor");
    if (ImGui::SliderFloat("##MinDistanceFactor", &minDistanceFactor, 0.0, 100.0f, "%.3f")) nmsBlock->SetMinDistanceThreshold(minDistanceFactor);
    TextCentered("Point Size");
    if (ImGui::SliderFloat("##PointSizeFactor", &pointSize, 0.0, 10.0f, "%.3f")) nmsBlock->SetPointSize(pointSize);
    SetItemWidth(default_color_wheel_size);
    if (ImGui::ColorPicker3("##PointColor", pointColor.data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_PickerHueWheel)) nmsBlock->SetPointColor(pointColor);
    ImGui::EndGroup();
    ImGui::PopID();

    ImGui::End();
  }
}

