#pragma once

#include "CameraTypes.h"

#ifdef _WIN32
#include <mfapi.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <linux/videodev2.h>
#else
#pragma message("(__FILE__): Not supported for unknown platform")
#endif

namespace gss::video::camera::formats {
#ifdef _WIN32
  const types::VideoFormat UYVY_FORMAT = MFVideoFormat_UYVY;
  const types::VideoFormat YUY2_FORMAT = MFVideoFormat_YUY2;
  const types::VideoFormat NV12_FORMAT = MFVideoFormat_NV12;
  const types::VideoFormat RGB_FORMAT = MFVideoFormat_RGB24;
  const types::VideoFormat RGBA_FORMAT = MFVideoFormat_RGB32;
  const types::VideoFormat MJPG_FORMAT = MFVideoFormat_MJPG;
#elif defined(__linux__) || defined(__APPLE__)
  constexpr types::VideoFormat UYVY_FORMAT = V4L2_PIX_FMT_UYVY;
  constexpr types::VideoFormat YUY2_FORMAT = V4L2_PIX_FMT_YUYV;
  constexpr types::VideoFormat NV12_FORMAT = V4L2_PIX_FMT_NV12;
  constexpr types::VideoFormat RGB_FORMAT = V4L2_PIX_FMT_RGB24;
  constexpr types::VideoFormat RGBA_FORMAT = V4L2_PIX_FMT_RGB32;
  constexpr types::VideoFormat MJPG_FORMAT = V4L2_PIX_FMT_MJPEG;
#else
  constexpr types::VideoFormat UYVY_FORMAT = 0;
  constexpr types::VideoFormat YUY2_FORMAT = 0;
  constexpr types::VideoFormat NV12_FORMAT = 0;
  constexpr types::VideoFormat RGB_FORMAT = 0;
  constexpr types::VideoFormat RGBA_FORMAT = 0;
  constexpr types::VideoFormat MJPG_FORMAT = 0;
#endif
}
