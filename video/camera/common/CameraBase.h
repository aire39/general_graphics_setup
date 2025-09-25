#pragma once

#include <memory>
#include "video/camera/common/CameraTypes.h"

class FImage;

namespace gss::video::camera {
  class CameraBase
  {
    public:
      virtual ~CameraBase() = default;
      virtual void StartCapture() = 0;
      virtual void PauseCapture() = 0;
      virtual void StopCapture() = 0;
      virtual void Reset() = 0;
      virtual void ChangeFramerate(gss::video::camera::types::FrameRate frame_rate, bool reset = true) = 0;
      virtual void ChangeResolution(gss::video::camera::types::FrameSize frame_size, bool reset = true) = 0;
      virtual void ChangePixelFormat(gss::video::camera::types::VideoFormat video_format, bool reset = true) = 0;
      virtual std::shared_ptr<FImage> ExtractFrame() = 0;
      virtual std::shared_ptr<FImage> CopyFrame() = 0;

      virtual gss::video::camera::types::FrameRate GetFramerate() const = 0;
      virtual gss::video::camera::types::FrameSize GetResolution() const = 0;
      virtual gss::video::camera::types::VideoFormat GetPixelFormat() const = 0;
      virtual float GetRunningFps() const = 0;
      virtual bool IsCapturing() const = 0;

    protected:
      virtual bool AllocateBuffers() = 0;
      virtual bool Configuration() = 0;
      virtual void CaptureFramesThread() = 0;

      bool isCapturing = false;
      bool isPause = false;
  };
}