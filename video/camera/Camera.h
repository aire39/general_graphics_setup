#pragma once

#ifdef _WIN32
#include "video/camera/platform/WinCamera.h"
#elif defined(__linux__) || defined(__unix__)
#include "video/camera/platform/LinuxCamera.h"
#else
#pragma message("Camera support: not available")
#endif

namespace gss::video::camera {
#ifdef _WIN32
  typedef gss::video::camera::platform::WinCamera Camera;
#elif defined(__linux__) || defined(__unix__)
  typedef gss::video::camera::platform::LinuxCamera Camera;
#else
#endif
}