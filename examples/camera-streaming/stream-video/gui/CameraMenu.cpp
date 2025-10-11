#include "CameraMenu.h"

#include <imgui.h>
#include <spdlog/spdlog.h>
#include "video/camera/common/CameraBase.h"
#include "video/camera/common/CameraHelpers.h"

namespace {
  constexpr bool update_on_change = false;
}

CameraMenu::CameraMenu(gss::video::camera::CameraBase *camera)
  : camera(camera)
  , frameRate{static_cast<int32_t>(camera->GetFramerate().first), static_cast<int32_t>(camera->GetFramerate().second)}
  , resolution{static_cast<int32_t>(camera->GetResolution().first), static_cast<int32_t>(camera->GetResolution().second)}
{
}

void CameraMenu::RenderMenu()
{
  ImGui::Begin("Camera Information", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::PushID(0);
  ImGui::BeginGroup();
  if(ImGui::Button("Information Tab")) option = MenuOptions::INFO;
  ImGui::SameLine();
  if(ImGui::Button("Settings Tab")) option = MenuOptions::SETTINGS;
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
  else // (option == MenuOptions::SETTINGS)
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
    ImGui::PushID(3);
    if (ImGui::Button("Start Capture")) {camera->StartCapture();}
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID(4);
    if (ImGui::Button("Stop Capture")) {camera->StopCapture();}
    ImGui::PopID();
    ImGui::EndGroup();
    ImGui::PopID();
  }

  ImGui::End();
}
