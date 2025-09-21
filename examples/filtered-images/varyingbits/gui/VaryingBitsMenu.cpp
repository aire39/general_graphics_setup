#include "VaryingBitsMenu.h"

#include <imgui.h>
#include <spdlog/spdlog.h>

VaryingBitsMenu::VaryingBitsMenu()
{
  for (auto & v : showBitPlaneOption)
  {
    v = true;
  }
}

void VaryingBitsMenu::RenderMenu()
{
  ImGui::Begin("Varying Bits Operation", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::BeginGroup();
  ImGui::Text("Bits Level:");
  ImGui::SliderInt("##bit_level", &setBit, 8, 1);
  ImGui::EndGroup();

  ImGui::NewLine();

  ImGui::BeginGroup();
  ImGui::TextColored(ImVec4(0.75f, 0.5f, 0.9f, 1.0f), "Bit Plane Mode:");
  if (ImGui::RadioButton("Bit Plane", showBitPlaneMode))
  {
    showBitPlaneMode = true;
    showHidePlaneMode = false;
    forceBitScaleUpdate = true;
  }

  ImGui::SameLine();

  if (ImGui::RadioButton("Show/Hide Plane(s)", showHidePlaneMode))
  {
    showHidePlaneMode = true;
    showBitPlaneMode = false;
    processBegin = true;
  }

  ImGui::EndGroup();

  bool has_changed = showBitPlaneOption[8 - (setBit)];

  ImGui::NewLine();
  ImGui::TextColored(ImVec4(0.75f, 0.5f, 0.9f, 1.0f), "Bit Mode Options");

  uint8_t selected_bit = (0x80 >> (8 - (setBit-1)));
  ImGui::PushID(selected_bit);

  if (showHidePlaneMode)
  {
    ImGui::Checkbox("show bit-plane", &showBitPlaneOption[8 - (setBit)]);

    ImGui::NewLine();

    if(ImGui::Button("Reset Bit Planes"))
    {
      for (auto & is_bit_plane_showing : showBitPlaneOption)
      {
        is_bit_plane_showing = true;
      }
      processBegin = true;
    }
  }

  if ((showBitPlaneOption[8 - (setBit)] != has_changed) && showHidePlaneMode)
  {
    processBegin = true;
  }

  ImGui::PopID();

  if (showBitPlaneMode)
  {
    bool tmp_shiftbitscontrast_changed = shiftBitsContrast;
    bool tmp_usecolor_changed = useColor;

    ImGui::Checkbox("contrast bit shift", &shiftBitsContrast);
    ImGui::Checkbox("use color channels", &useColor);

    if ((tmp_shiftbitscontrast_changed != shiftBitsContrast) || (tmp_usecolor_changed != useColor))
    {
      forceBitScaleUpdate = true;
    }
  }

  ImGui::NewLine();

  ImGui::Text("note: move slider if image doesn't show");

  ImGui::End();
}

int32_t VaryingBitsMenu::BitScale() const
{
  return setBit;
}

bool VaryingBitsMenu::ShiftBitsForContrast() const
{
  return shiftBitsContrast;
}

bool VaryingBitsMenu::ProcessBegin()
{
  bool should_process = processBegin;
  if (should_process)
  {
    processBegin = false;
  }

  return should_process;
}

bool VaryingBitsMenu::UseColorChannels() const
{
  return useColor;
}

const std::array<bool, 8> & VaryingBitsMenu::ShowBitPlanes() const
{
  return showBitPlaneOption;
}

uint8_t VaryingBitsMenu::BitPlaneMask() const
{
  uint8_t mask = 0xFF;
  for (int32_t i = 7; i >= 0; i--)
  {
    mask = (~(1 << i) & mask);
    mask = ((showBitPlaneOption[(showBitPlaneOption.size()-1) - i] << i) | mask);
  }

  return mask;
}

bool VaryingBitsMenu::ShowHidePlaneMode() const
{
  return showHidePlaneMode;
}

bool VaryingBitsMenu::ForceBitScaleUpdate()
{
  bool tmp_force = forceBitScaleUpdate;
  forceBitScaleUpdate = false;
  return tmp_force;
}