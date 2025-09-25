#pragma once

#include <cstdint>
#include <utility>

#if defined(_WIN32) || defined(__MINGW64__)
#include <guiddef.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <cstdint>
#else
#include <cstdint>
#endif


namespace gss::video::camera::types {
#if defined(_WIN32) || defined(__MINGW64__)
  typedef GUID VideoFormat;
#elif defined(__linux__) || defined(__APPLE__)
  typedef uint32_t VideoFormat;
#else
  typedef uint32_t VideoFormat;
#endif

  typedef std::pair<uint32_t, uint32_t> FrameRate;
  typedef std::pair<uint32_t, uint32_t> FrameSize;
}
