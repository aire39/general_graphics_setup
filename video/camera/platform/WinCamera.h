#pragma once

#include "../common/CameraBase.h"
#include "../common/CameraTypes.h"
#include "common/cthreads/cthread.h"

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>
#include <mfobjects.h>

#include <cstdint>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <queue>

class FImage;

namespace gss::video::camera::platform {
  class WinCamera final : public CameraBase
  {
    public:
      WinCamera();
      explicit WinCamera(uint32_t capture_device_index, int32_t width, int32_t height, const gss::video::camera::types::FrameRate &fps, gss::video::camera::types::VideoFormat video_format, bool ignore_fail_format = false);
      explicit WinCamera(const std::string &capture_device_path, int32_t width, int32_t height, const gss::video::camera::types::FrameRate &fps, gss::video::camera::types::VideoFormat video_format, bool ignore_fail_format = false);
      ~WinCamera() override;

      void StartCapture() override;
      void PauseCapture() override;
      void StopCapture() override;
      void Reset() override;
      void ChangeFramerate(gss::video::camera::types::FrameRate frame_rate, bool reset = true) override;
      void ChangeResolution(gss::video::camera::types::FrameSize frame_size, bool reset = true) override;
      void ChangePixelFormat(gss::video::camera::types::VideoFormat video_format, bool reset = true) override;
      std::shared_ptr<FImage> ExtractFrame() override;
      std::shared_ptr<FImage> CopyFrame() override;

      gss::video::camera::types::FrameRate GetFramerate() const override;
      gss::video::camera::types::FrameSize GetResolution() const override;
      gss::video::camera::types::VideoFormat GetPixelFormat() const override;
      float GetRunningFps() const override;
      bool IsCapturing() const override;

    protected:
      bool AllocateBuffers() override;
      bool Configuration() override;
      void CaptureFramesThread() override;
      void SampleFramesThread();

    private:
      int32_t FindCaptureDeviceByPath(std::string_view path);
      std::string FindCaptureDeviceByPath(int32_t capture_device_index);

      uint32_t captureDevice = 0;
      uint32_t numSources = 0;
      IMFActivate** devices = nullptr;
      IMFMediaSource* source = nullptr;
      IMFSourceReader* reader = nullptr;
      IMFAttributes* attributes = nullptr;
      uint32_t failCount = 0;
      uint32_t frameCount = 0;
      int32_t width = 640;
      int32_t height = 480;
      gss::video::camera::types::FrameRate fps = {30, 1};
      double runningFrameTime = 0.0;
      bool isInitialized = false;
      bool ignoreFailFormat = false;
      std::string devicePath;
      gss::video::camera::types::VideoFormat videoFormat;

      std::queue<std::tuple<Microsoft::WRL::ComPtr<IMFSample>, DWORD, DWORD, LONGLONG>> sampleBufferQueue;
      cthread captureThread;
      cthread sampleThread;
      std::mutex mutexCaptureFrame;
      std::mutex mutexQueue;
      std::mutex mutexReadyQueue;
      std::mutex mutexCaptureQueue;
      std::condition_variable cvCaptureFrame;

      std::vector<std::shared_ptr<FImage>> frames;
      std::queue<std::shared_ptr<FImage>> captureFrameQueue;
      std::queue<std::shared_ptr<FImage>> readyFrameQueue;
  };
}
