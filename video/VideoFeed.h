#pragma once

#include "video/camera/common/CameraBase.h"

class VideoFeed final : public gss::video::camera::CameraBase
{
  public:
    VideoFeed();
    ~VideoFeed() override;

    void StartCapture() override;
    void PauseCapture() override;
    void StopCapture() override;
    void Reset() override;
    void ChangeFramerate(gss::video::camera::types::FrameRate frame_rate, bool reset = true) override;
    void ChangeResolution(gss::video::camera::types::FrameSize frame_size, bool reset = true) override;
    void ChangePixelFormat(gss::video::camera::types::VideoFormat video_format, bool reset = true) override;
    void IgnoreFormatFail(bool ignore, bool reset = true) override;
    std::shared_ptr<FImage> ExtractFrame() override;
    std::shared_ptr<FImage> CopyFrame() override;

    gss::video::camera::types::FrameRate GetFramerate() const override;
    gss::video::camera::types::FrameSize GetResolution() const override;
    gss::video::camera::types::VideoFormat GetPixelFormat() const override;
    float GetRunningFps() const override;
    bool IsCapturing() const override;

    uint32_t GetFailCount() const override;
    uint32_t GetSkippedFrames() const override;
    uint32_t GetMaxBufers() const override;
    uint32_t GetFrameCount() const override;

  protected:
    bool AllocateBuffers() override;
    bool Configuration() override;
    void CaptureFramesThread() override;
};
