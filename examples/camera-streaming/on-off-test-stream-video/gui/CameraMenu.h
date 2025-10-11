#pragma once

#include <cstdint>

namespace gss::video::camera{class CameraBase;}

class CameraMenu
{
  public:
    CameraMenu() = delete;
    explicit CameraMenu(gss::video::camera::CameraBase* camera);
    ~CameraMenu() = default;

    void RenderMenu() const;

private:
    gss::video::camera::CameraBase* camera;
    int32_t frameRate[2] {};
    int32_t resolution[2] {};
    int32_t videoFormat = 0;
    bool ignoreFormatFail = true;
};
