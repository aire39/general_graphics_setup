#include "PixelCornersMenu.h"

#include <imgui.h>
#include "process/PixelCornersBlock.h"

PixelCornersMenu::PixelCornersMenu(PixelCornersBlock *pixel_block)
  : pixelBlock(pixel_block)
{
}

void PixelCornersMenu::RenderMenu()
{
  ImGui::Begin("Pixel Corners", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  if (pixelBlock)
  {
    ImGui::PushID(1);
    ImGui::BeginGroup();
    if (ImGui::SliderFloat("K Factor", &kFactor, 0.01f, 0.1f, "%.2f")) pixelBlock->SetKValue(kFactor);
    if (ImGui::SliderFloat("Sigma Factor", &sigmaFactor, 0.0, 1.0f, "%.3f")) pixelBlock->SetSigma(sigmaFactor);
    ImGui::EndGroup();
    ImGui::PopID();

    ImGui::End();
  }
}
