#pragma once

#include <cstdint>
#include "video/camera/common/CameraTypes.h"

namespace gss::video::camera{class CameraBase;}
class PixelFormatConversionBlock;

class CameraMenu
{
  public:
    CameraMenu() = delete;
    explicit CameraMenu(gss::video::camera::CameraBase* camera, PixelFormatConversionBlock* pixel_format_conversion_block);
    ~CameraMenu() = default;

    void RenderMenu();
    gss::video::camera::types::VideoFormat GetFormatConversionType() const;

private:
    enum class MenuOptions {INFO, SETTINGS};

    MenuOptions option = MenuOptions::INFO;
    gss::video::camera::CameraBase* camera = nullptr;
    PixelFormatConversionBlock* pixelFormatConversionBlock = nullptr;
    int32_t frameRate[2] {};
    int32_t resolution[2] {};
    int32_t videoFormat = 0;
    bool ignoreFormatFail = true;
    uint32_t id = 0;

    inline static uint32_t refCount = 0;
};
