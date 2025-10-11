#pragma once

#include "CameraFormats.h"
#include "spdlog/spdlog.h"

#include <string>
#include <fstream>

namespace gss::video::camera::helpers {

  inline gss::video::camera::types::VideoFormat get_format_by_index(const int index)
  {
    types::VideoFormat format;
    switch (index)
    {
      case 0: format = gss::video::camera::formats::UYVY_FORMAT; break;
      case 1: format = gss::video::camera::formats::YUY2_FORMAT; break;
      case 2: format = gss::video::camera::formats::NV12_FORMAT; break;
      case 3: format = gss::video::camera::formats::RGB_FORMAT; break;
      case 4: format = gss::video::camera::formats::RGBA_FORMAT; break;
      case 5: format = gss::video::camera::formats::MJPG_FORMAT; break;
      default: format = gss::video::camera::formats::UYVY_FORMAT; break;
    }

    return format;
  }

  inline gss::video::camera::types::VideoFormat get_format_by_index(const std::string &str_index)
  {
    int32_t index = 0;

    try
    {
      index = std::stoi(str_index);
    }
    catch (const std::invalid_argument& e)
    {
      index = 0;
      spdlog::warn("Could not determine format selection. Will default to UYVY --> {} failed to convert string to integer: {}", e.what(), str_index);
    }

    gss::video::camera::types::VideoFormat format;
    switch (index)
    {
      case 0: format = gss::video::camera::formats::UYVY_FORMAT; break;
      case 1: format = gss::video::camera::formats::YUY2_FORMAT; break;
      case 2: format = gss::video::camera::formats::NV12_FORMAT; break;
      case 3: format = gss::video::camera::formats::RGB_FORMAT; break;
      case 4: format = gss::video::camera::formats::RGBA_FORMAT; break;
      case 5: format = gss::video::camera::formats::MJPG_FORMAT; break;
      default: format = gss::video::camera::formats::UYVY_FORMAT; break;
    }

    return format;
  }

  #if defined(_WIN32)

  inline std::string get_video_format_str(const gss::video::camera::types::VideoFormat format)
  {
    std::string format_str;

    if (IsEqualGUID(format, gss::video::camera::formats::UYVY_FORMAT))
    {
      format_str = "UYVY";
    }
    else if (IsEqualGUID(format, gss::video::camera::formats::YUY2_FORMAT))
    {
      format_str = "YUY2";
    }
    else if (IsEqualGUID(format, gss::video::camera::formats::NV12_FORMAT))
    {
      format_str = "NV12";
    }
    else if (IsEqualGUID(format, gss::video::camera::formats::RGB_FORMAT))
    {
      format_str = "RGB";
    }
    else if (IsEqualGUID(format, gss::video::camera::formats::RGBA_FORMAT))
    {
      format_str = "RGBA";
    }
    else if (IsEqualGUID(format, gss::video::camera::formats::MJPG_FORMAT))
    {
      format_str = "MJPG";
    }
    else
    {
      format_str = "UYVY";
    }

    return format_str;
  }
#elif defined(__linux__) || defined(__unix__)
  inline std::string get_video_format_str(const gss::video::camera::types::VideoFormat format)
  {
    std::string format_str;

    switch (format)
    {
      case gss::video::camera::formats::UYVY_FORMAT:
      default:
        format_str = "UYVY";
        break;
    }

    return format_str;
  }
#endif
}
