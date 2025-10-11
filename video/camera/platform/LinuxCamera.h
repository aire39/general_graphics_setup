#pragma once

#include "common/cthreads/cthread.h"
#include "video/camera/common/CameraBase.h"
#include "video/camera/common/CameraTypes.h"

#include <cstdint>
#include <string>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <queue>

class FImage;

namespace gss::video::camera::platform {
  class LinuxCamera final : public gss::video::camera::CameraBase
  {
    public:
      LinuxCamera();
      explicit LinuxCamera(uint32_t capture_device_index, int32_t width, int32_t height, const gss::video::camera::types::FrameRate &fps, gss::video::camera::types::VideoFormat format, bool ignore_fail_format = false);
      explicit LinuxCamera(const std::string &device_path, int32_t width, int32_t height, const gss::video::camera::types::FrameRate &fps, gss::video::camera::types::VideoFormat format, bool ignore_fail_format = false);
      ~LinuxCamera() override;

      void StartCapture() override;
      void PauseCapture() override;
      void StopCapture() override;
      void Reset() override;
      void ChangeFramerate(gss::video::camera::types::FrameRate frame_rate, bool reset) override;
      void ChangeResolution(gss::video::camera::types::FrameSize frame_size, bool reset) override;
      void ChangePixelFormat(gss::video::camera::types::VideoFormat video_format, bool reset) override;
      void IgnoreFormatFail(bool ignore, bool reset = true) override;
      std::shared_ptr<FImage> ExtractFrame() override;
      std::shared_ptr<FImage> CopyFrame() override;

      gss::video::camera::types::FrameRate GetFramerate() const override;
      gss::video::camera::types::FrameSize GetResolution() const override;
      gss::video::camera::types::VideoFormat GetPixelFormat() const override;
      float GetRunningFps() const override;
      bool IsCapturing() const override;

      uint32_t GetFailCount() const;
      uint32_t GetSkippedFrames() const;
      uint32_t GetMaxBufers() const;
      uint32_t GetFrameCount() const;

    protected:
      bool AllocateBuffers() override;
      bool Configuration() override;
      void CaptureFramesThread() override;

    private:
      bool WaitForFrame();
      bool QueueFrame(int32_t index);

      int32_t fd = -1;
      uint32_t captureDevice = 0;
      uint32_t failCount = 0;
      uint32_t notActiveCount = 0;
      uint32_t frameCount = 0;
      uint32_t queuedBuffers = 0;
      int32_t width = 640;
      int32_t s_width = 640;
      int32_t height = 480;
      int32_t s_height = 480;
      gss::video::camera::types::FrameRate fps = {30, 1};
      gss::video::camera::types::FrameRate s_fps = {30, 1};
      double runningFrameTime = 0.0;
      bool isInitialized = false;
      bool ignoreFailFormat = false;
      bool s_ignoreFailFormat = false;
      std::string devicePath = "/dev/video0";
      uint32_t skipFrames = 0;
      gss::video::camera::types::VideoFormat videoFormat;
      gss::video::camera::types::VideoFormat s_videoFormat;

      cthread captureThread;
      cthread sampleThread;
      std::mutex mutexCaptureFrame;
      std::mutex mutexReadyQueue;
      std::mutex mutexQueueTrack;
      std::condition_variable cvCaptureFrame;

      std::vector<std::pair<uint8_t*, uint32_t>> buffers;
      std::vector<std::shared_ptr<FImage>> frames;
      std::queue<std::shared_ptr<FImage>> readyFrameQueue;
  };
}
