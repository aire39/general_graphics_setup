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

void CameraMenu::RenderMenu() const
{
  ImGui::Begin("Camera Information", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

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

  ImGui::End();
}
