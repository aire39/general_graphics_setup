#include "CameraMenu.h"

#include <imgui.h>
#include <spdlog/spdlog.h>
#include "video/camera/common/CameraBase.h"
#include "video/camera/common/CameraHelpers.h"
#include "graphics/images/FImage.h"
#include "../filters/EdgeFilters.h"
#include "support/logging.h"

namespace {
  constexpr bool update_on_change = false;
  constexpr bool save_filter = true;

#if defined(__linux__) || defined(__unix__)
  constexpr bool enable_random_filter_for_linux = false;
#else
  constexpr bool enable_random_filter_for_linux = true;
#endif
}

CameraMenu::CameraMenu(gss::video::camera::CameraBase *camera)
  : camera(camera)
  , frameRate{static_cast<int32_t>(camera->GetFramerate().first), static_cast<int32_t>(camera->GetFramerate().second)}
  , resolution{static_cast<int32_t>(camera->GetResolution().first), static_cast<int32_t>(camera->GetResolution().second)}
{
  filters = {{
       {"Normal", filter::functions::cpu::parallel_vectorize::default_filter_process}
      ,{"Grayscale", filter::functions::cpu::parallel_vectorize::convert_to_grayscale}
      ,{"Random", filter::functions::cpu::parallel_vectorize::random_pixel_colors}
      ,{"Blur", filter::functions::cpu::parallel_vectorize::gaussian_blur_3x3}
      ,{"Edge", filter::functions::cpu::parallel_vectorize::edge_process}
      ,{"Grayscale Edge", filter::functions::cpu::parallel_vectorize::edge_process}
    }};
}

void CameraMenu::UpdateFrame(FImage *image) const
{
  if (image)
  {
    if (selectedFilterIndex == 5)
    {
      image->ProcessFilter(filters[1].second, save_filter);
      image->ProcessFilter(selectedFilter, save_filter);
    }
    else
    {
      image->ProcessFilter(selectedFilter, save_filter);
    }
  }
}

void CameraMenu::RenderMenu()
{
  ImGui::Begin("Camera Information", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::PushID(0);
  ImGui::BeginGroup();
  if(ImGui::Button("Information Tab")) option = MenuOptions::INFO;
  ImGui::SameLine();
  if(ImGui::Button("Settings Tab")) option = MenuOptions::SETTINGS;
  ImGui::SameLine();
  if(ImGui::Button("Filters Tab")) option = MenuOptions::FILTERS;
  ImGui::EndGroup();
  ImGui::PopID();

  if (option == MenuOptions::INFO)
  {
    ImGui::PushID(1);
    ImGui::BeginGroup();
    ImGui::TextUnformatted(("Framerate: " + (std::to_string(camera->GetFramerate().first) + "/" + std::to_string(camera->GetFramerate().second))).c_str());
    ImGui::TextUnformatted(("Frame Resolution: " + std::to_string(camera->GetResolution().first) + "x" + std::to_string(camera->GetResolution().second)).c_str());
    ImGui::TextUnformatted(("Pixel Format: " + gss::video::camera::helpers::get_video_format_str(camera->GetPixelFormat())).c_str());\
    ImGui::TextUnformatted(("Framerate (FPS): " + std::to_string(camera->GetRunningFps())).c_str());
    ImGui::TextUnformatted(("Frame Count: " + std::to_string(camera->GetFrameCount())).c_str());
    ImGui::TextUnformatted(("Fail Frame Count: " + std::to_string(camera->GetFailCount())).c_str());
    ImGui::TextUnformatted(("Skip Frames: " + std::to_string(camera->GetSkippedFrames())).c_str());
    ImGui::TextUnformatted(("Max Buffers: " + std::to_string(camera->GetMaxBufers())).c_str());
    ImGui::EndGroup();
    ImGui::PopID();
  }
  else if (option == MenuOptions::SETTINGS)
  {
    ImGui::PushID(2);
    ImGui::BeginGroup();
    ImGui::TextUnformatted("Set Framerate:");
    if (ImGui::InputInt2("##framerate", frameRate)) {camera->ChangeFramerate({frameRate[0], frameRate[1]}, update_on_change);}
    ImGui::TextUnformatted("Set Resolution:");
    if (ImGui::InputInt2("##resolution", resolution)) {camera->ChangeResolution({resolution[0], resolution[1]}, update_on_change);}
    ImGui::TextUnformatted("Set Pixel Format:");
    if (ImGui::InputInt("##video-format", &videoFormat)) {camera->ChangePixelFormat(gss::video::camera::helpers::get_format_by_index(videoFormat), update_on_change);}
    if (ImGui::Checkbox("ignore format fail", &ignoreFormatFail)) {camera->IgnoreFormatFail(ignoreFormatFail, update_on_change);}

    const float spacing = ImGui::GetStyle().ItemSpacing.x;  // Space between buttons
    const float full_width = ImGui::GetContentRegionAvail().x;
    const float button_width = (full_width - spacing) / 2.0f;

    ImGui::PushID(3);
    if (ImGui::Button("Start Capture", ImVec2(button_width, 0))) {camera->StartCapture();}
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID(4);
    if (ImGui::Button("Stop Capture", ImVec2(button_width, 0))) {camera->StopCapture();}
    ImGui::PopID();
    ImGui::EndGroup();
    ImGui::PopID();
  }
  else // (option == MenuOptions::FILTERS)
  {
    ImGui::PushID(5);
    ImGui::BeginGroup();

    ImGui::TextUnformatted("Select Filter");
    if (ImGui::Combo("##filter-listbox", &selectedFilterIndex, FilterListGetter, &filters, static_cast<int32_t>(filters.size())))
    {
      if constexpr (!enable_random_filter_for_linux)
      {
        // ReSharper disable once CppDFAUnreachableCode NOLINTNEXTLINE(clang-diagnostic-unreachable-code)
        if (selectedFilterIndex == 2)
        {
          logging::warn("Filter not runnable for the moment!");
        }
        else
        {
          selectedFilter = filters[selectedFilterIndex].second;
        }
      }
      else
      {
        // ReSharper disable once CppDFAUnreachableCode NOLINTNEXTLINE(clang-diagnostic-unreachable-code)
        selectedFilter = filters[selectedFilterIndex].second;
      }
    }
    ImGui::EndGroup();
    ImGui::PopID();
  }

  ImGui::End();
}

bool CameraMenu::FilterListGetter(void *filter_ptr, int32_t index, const char **text)
{
  bool found = true;

  auto* items = static_cast<FilterList*>(filter_ptr);
  if (index < 0 || index >= static_cast<int32_t>(items->size()))
  {
    found = false;
  }

  if (found)
  {
    *text = items->at(index).first.c_str();
  }

  return found;
}
