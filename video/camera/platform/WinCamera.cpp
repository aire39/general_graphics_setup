#include "WinCamera.h"

#include <string>
#include <chrono>

#include "graphics/FImage.h"
#include "SDL3/SDL_surface.h"
#include "spdlog/spdlog.h"

#include "common/support/utility.h"
#include "video/camera/common/CameraFormats.h"
#include "graphics/Filters.h"
#include "spdlog/fmt/bundled/os.h"

namespace {
  constexpr uint32_t max_capture_fails = 5;
  constexpr uint32_t default_device_index = 0;
  constexpr uint32_t default_frame_width = 640;
  constexpr uint32_t default_frame_height = 480;
  constexpr uint32_t default_number_of_buffers = 8;
  constexpr bool default_ignore_fail_format = false;
  constexpr double lpf_smooth_factor = 0.1;
  constexpr gss::video::camera::types::FrameRate default_frame_fps = {30, 1};
}

#ifndef MF_LOW_LATENCY
DEFINE_GUID(MF_LOW_LATENCY, 0x9c27891a, 0xed7a, 0x40e1, 0x88, 0xe8, 0xb2, 0x27, 0x27, 0xa0, 0x24, 0xee);
#endif

namespace gss::video::camera::platform {
  WinCamera::WinCamera()
    : WinCamera(default_device_index, default_frame_width, default_frame_height, default_frame_fps, gss::video::camera::formats::UYVY_FORMAT, default_ignore_fail_format)
  {
  }

  WinCamera::WinCamera(const uint32_t capture_device_index, int32_t width, int32_t height, const gss::video::camera::types::FrameRate &fps, const gss::video::camera::types::VideoFormat video_format, bool ignore_fail_format)
    :captureDevice (capture_device_index)
    ,width(width)
    ,s_width(width)
    ,height(height)
    ,s_height(height)
    ,fps(fps)
    ,s_fps(fps)
    ,ignoreFailFormat(ignore_fail_format)
    ,s_ignoreFailFormat(ignoreFailFormat)
    ,videoFormat(video_format)
    ,s_videoFormat(videoFormat)
  {
    devicePath = FindCaptureDeviceByPath(capture_device_index);
  }

  WinCamera::WinCamera(const std::string &capture_device_path, int32_t width, int32_t height, const gss::video::camera::types::FrameRate &fps, gss::video::camera::types::VideoFormat video_format, bool ignore_fail_format)
    :width(width)
    ,s_width(width)
    ,height(height)
    ,s_height(height)
    ,fps(fps)
    ,s_fps(fps)
    ,ignoreFailFormat(ignore_fail_format)
    ,s_ignoreFailFormat(ignoreFailFormat)
    ,devicePath(capture_device_path)
    ,videoFormat(video_format)
    ,s_videoFormat(videoFormat)
  {
    captureDevice = FindCaptureDeviceByPath(capture_device_path.c_str());
  }

  WinCamera::~WinCamera()
  {
    WinCamera::StopCapture();
  }

  void WinCamera::StartCapture()
  {
    width = s_width;
    height = s_height;
    fps = s_fps;
    videoFormat = s_videoFormat;
    ignoreFailFormat = s_ignoreFailFormat;

    if (isPause && isCapturing)
    {
      isPause = false;
    }
    else
    {
      if (!isCapturing)
      {
        if (AllocateBuffers())
        {
          if ((isInitialized = WinCamera::Configuration() || ignoreFailFormat))
          {
            isCapturing = true;
            spdlog::info("Starting capture...");
            captureThread = cthread("cap-camera", "capture frame data", &WinCamera::CaptureFramesThread, this);
            sampleThread = cthread("samp-camera", "capture frame data", &WinCamera::SampleFramesThread, this);
          }
          else
          {
            spdlog::warn("Video device is not initialized");
          }
        }
        else
        {
          spdlog::warn("Failed to allocate buffers. cannot start camera!");
        }
      }
      else
      {
        spdlog::info("Capture already started!");
      }
    }
  }

  void WinCamera::PauseCapture()
  {
    isPause = true;
  }

  void WinCamera::StopCapture()
  {
    PauseCapture();
    isCapturing = false;

    if (sampleThread.joinable())
    {
      sampleThread.join();
    }

    {
      std::lock_guard lock_sample_queue(mutexQueue);
      while (!sampleBufferQueue.empty())
      {
        sampleBufferQueue.pop();
      }
    }

    if (captureThread.joinable())
    {
      captureThread.join();
    }

    frames = std::vector<std::shared_ptr<FImage>>();

    if (reader)
    {
      reader->Release();
      reader = nullptr;
    }

    if (attributes)
    {
      attributes->Release();
      attributes = nullptr;
    }

    if (source)
    {
      source->Release();
      source = nullptr;
    }

    if (devices && numSources > 0)
    {
      for (size_t i=0; i<numSources; i++)
      {
        devices[i]->Release();
      }
    }

    if (devices)
    {
      CoTaskMemFree(devices);
      devices = nullptr;
    }

    HRESULT hr = MFShutdown();
    if (FAILED(hr))
    {
      spdlog::error("Failed to shutdown Media Foundation For Video Capture");
    }

    isPause = false;
  }

  void WinCamera::Reset()
  {
    StopCapture();
    StartCapture();
  }

  void WinCamera::ChangeFramerate(gss::video::camera::types::FrameRate frame_rate, bool reset)
  {
    if (s_fps.first != frame_rate.first || s_fps.second != frame_rate.second)
    {
      s_fps = frame_rate;
      if (reset)
      {
        Reset();
      }
    }
  }

  void WinCamera::ChangeResolution(gss::video::camera::types::FrameSize frame_size, bool reset)
  {
    if (s_width != static_cast<int32_t>(frame_size.first) || s_height != static_cast<int32_t>(frame_size.second))
    {
      s_width = frame_size.first;
      s_height = frame_size.second;
      if (reset)
      {
        Reset();
      }
    }
  }

  void WinCamera::ChangePixelFormat(gss::video::camera::types::VideoFormat video_format, bool reset)
  {
    if (s_videoFormat != video_format)
    {
      s_videoFormat = video_format;
      if (reset)
      {
        Reset();
      }
    }
  }

  void WinCamera::IgnoreFormatFail(bool ignore, bool reset)
  {
    if (s_ignoreFailFormat != ignore)
    {
      s_ignoreFailFormat = ignore;
      if (reset)
      {
        Reset();
      }
    }
  }

  std::shared_ptr<FImage> WinCamera::ExtractFrame()
  {
    std::lock_guard ready_lock(mutexReadyQueue);
    std::shared_ptr<FImage> image = nullptr;

    if (isCapturing)
    {
      if (!readyFrameQueue.empty())
      {
        image = readyFrameQueue.front();
        readyFrameQueue.pop();
      }
    }

    return image;
  }

  std::shared_ptr<FImage> WinCamera::CopyFrame()
  {
    std::lock_guard ready_lock(mutexReadyQueue);
    std::shared_ptr<FImage> image = nullptr;

    if (isCapturing)
    {
      if (!readyFrameQueue.empty())
      {
        const std::shared_ptr<FImage> frame = readyFrameQueue.front();
        image = std::make_shared<FImage>(*frame.get());
        readyFrameQueue.pop();
      }
    }

    return image;
  }

  gss::video::camera::types::FrameRate WinCamera::GetFramerate() const
  {
    return fps;
  }

  gss::video::camera::types::FrameSize WinCamera::GetResolution() const
  {
    return {width, height};
  }

  gss::video::camera::types::VideoFormat WinCamera::GetPixelFormat() const
  {
    return videoFormat;
  }

  float WinCamera::GetRunningFps() const
  {
    return 1.0f / static_cast<float>(runningFrameTime);
  }

  bool WinCamera::IsCapturing() const
  {
    return isCapturing;
  }

  uint32_t WinCamera::GetFailCount() const
  {
    return failCount;
  }

  uint32_t WinCamera::GetSkippedFrames() const
  {
    return skipFrames;
  }

  uint32_t WinCamera::GetMaxBufers() const
  {
    return default_number_of_buffers;
  }

  uint32_t WinCamera::GetFrameCount() const
  {
    return frameCount;
  }

  bool WinCamera::AllocateBuffers()
  {
    decltype(filter::functions::cpu::parallel_vectorize::uyvy_to_rgb_conversion) use_filter;
    if (videoFormat == gss::video::camera::formats::UYVY_FORMAT)
    {
      use_filter = filter::functions::cpu::parallel_vectorize::uyvy_to_rgb_conversion;
    }
    else if (videoFormat == gss::video::camera::formats::YUY2_FORMAT)
    {
      use_filter = filter::functions::cpu::parallel_vectorize::yuy2_to_rgb_conversion;
    }
    else
    {
      use_filter = filter::functions::cpu::parallel_vectorize::uyvy_to_rgb_conversion;
    }

    for (size_t i=0; i<default_number_of_buffers; i++)
    {
      constexpr bool base_same_as_orig = true;
      constexpr bool save_filter = true;
      std::string frame_name = "frame-" + std::to_string(i);

      auto new_fimage = std::make_shared<FImage>(frame_name, width, height, SDL_PixelFormat::SDL_PIXELFORMAT_RGB24, base_same_as_orig);
      new_fimage->ProcessFilter(use_filter, save_filter);
      frames.push_back(new_fimage);

      auto q_fimage = std::shared_ptr<FImage>(new_fimage.get(), []([[maybe_unused]] FImage* image){});
      captureFrameQueue.push(q_fimage);
    }

    return true;
  }

  bool WinCamera::Configuration()
  {
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
    {
      spdlog::error("Failed to initialize MFStartup");
      return false;
    }

    hr = MFCreateAttributes(&attributes, 1);
    if (FAILED(hr))
    {
      spdlog::error("Failed to create attributes");
      return false;
    }

    hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr))
    {
      spdlog::error("Failed to set VIDCAP_GUID");
      return false;
    }

    hr = MFEnumDeviceSources(attributes, &devices, &numSources);
    if (FAILED(hr) || numSources == 0)
    {
      spdlog::error("Failed to enumerate devices sources");
      return false;
    }

    hr = devices[captureDevice]->ActivateObject(__uuidof(IMFMediaSource), reinterpret_cast<void **>(&source));
    if (FAILED(hr))
    {
      spdlog::error("Failed to activate media source");
      return false;
    }

    hr = MFCreateSourceReaderFromMediaSource(source, attributes, &reader);
    if (FAILED(hr))
    {
      spdlog::error("Failed to read source from media source");
      return false;
    }

    hr = reader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), TRUE);
    if (FAILED(hr))
    {
      spdlog::error("Failed to set stream selection");
      return false;
    }

    IMFMediaType* media_type = nullptr;
    hr = MFCreateMediaType(&media_type);
    if (FAILED(hr))
    {
      spdlog::error("Failed to create media type");
      return false;
    }

    hr = media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (FAILED(hr))
    {
      spdlog::error("Failed to set major type");
      return false;
    }

    hr = media_type->SetGUID(MF_MT_SUBTYPE, videoFormat);
    if (FAILED(hr))
    {
      spdlog::error("Failed to set subtype of video format");
      return false;
    }

    constexpr UINT32 low_latency = TRUE;
    hr = attributes->SetUINT32(MF_LOW_LATENCY, low_latency);
    if (FAILED(hr))
    {
      spdlog::info("Failed to set low latency mode");
      return false;
    }

    hr = attributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, TRUE);
    if (FAILED(hr))
    {
      spdlog::info("Failed to disable converters");
      return false;
    }

    hr = attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, FALSE);
    if (FAILED(hr))
    {
      spdlog::info("Failed to disable video processing");
      return false;
    }

    hr = MFSetAttributeRatio(media_type, MF_MT_FRAME_RATE, fps.first, fps.second);
    if (FAILED(hr))
    {
      spdlog::error("Failed to get native media type");
      return false;
    }

    hr = MFSetAttributeSize(media_type, MF_MT_FRAME_SIZE, width, height);
    if (FAILED(hr))
    {
      spdlog::error("Failed to get native media type");
      return false;
    }

    hr = reader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), nullptr, media_type);
    if (FAILED(hr))
    {
      spdlog::warn("Failed to source video stream. Will fallback to closest meda type!");
      return false;
    }

    media_type->Release();
    spdlog::info("Camera device is initialized");

    return true;
  }

  void WinCamera::CaptureFramesThread()
  {
    spdlog::info("Capture frames!");

    double filtered_timestamp = 0;
    double previous_filtered_timestamp = 0;

    while (isCapturing)
    {
      if (isPause)
      {
        std::lock_guard lock_sample_queue(mutexQueue);
        while (!sampleBufferQueue.empty())
        {
          auto sample = std::get<Microsoft::WRL::ComPtr<IMFSample>>(sampleBufferQueue.front());
          const double current_timestamp = static_cast<double>(std::get<long long>(sampleBufferQueue.front()));

          if (filtered_timestamp != 0)
          {
            filtered_timestamp = (lpf_smooth_factor * current_timestamp) + (1.0 - lpf_smooth_factor) * filtered_timestamp;
            runningFrameTime = (filtered_timestamp - previous_filtered_timestamp) / 1e7; // the timestamps are in 100 nanosecond units and this should convert to fraction seconds
            previous_filtered_timestamp = filtered_timestamp;
          }
          else
          {
            filtered_timestamp = current_timestamp;
            previous_filtered_timestamp = filtered_timestamp;
          }

          sample.Reset();
          sampleBufferQueue.pop();
        }
      }

      std::unique_lock lock_sample_queue_check(mutexQueue);
      const bool does_sample_exist = !sampleBufferQueue.empty();
      lock_sample_queue_check.unlock();

      if (does_sample_exist)
      {
        //DWORD stream_index;
        //DWORD stream_flags;
        //LONGLONG timestamp;
        Microsoft::WRL::ComPtr<IMFSample> sample;

        {
          std::lock_guard lock_sample_queue(mutexQueue);
          sample = std::get<Microsoft::WRL::ComPtr<IMFSample>>(sampleBufferQueue.front());
          const double current_timestamp = static_cast<double>(std::get<LONGLONG>(sampleBufferQueue.front()));
          sampleBufferQueue.pop();

          if (filtered_timestamp != 0)
          {
            filtered_timestamp = (lpf_smooth_factor * current_timestamp) + (1.0 - lpf_smooth_factor) * filtered_timestamp;
            runningFrameTime = (filtered_timestamp - previous_filtered_timestamp) / 1e7;
            previous_filtered_timestamp = filtered_timestamp;
          }
          else
          {
            filtered_timestamp = current_timestamp;
            previous_filtered_timestamp = filtered_timestamp;
          }
        }

        if (!sample.Get())
        {
          sample.Reset();
          continue;
        }

        BYTE* buffer_data = nullptr;
        DWORD max_length = 0;
        DWORD current_length = 0;
        failCount = 0;

        Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;
        HRESULT hr = sample->ConvertToContiguousBuffer(buffer.GetAddressOf());
        if (FAILED(hr))
        {
          spdlog::warn("Failed to convert sample");
          buffer.Reset();
          failCount++;
          continue;
        }

        if (buffer.Get() != nullptr)
        {
          if (!captureFrameQueue.empty())
          {
            std::shared_ptr<FImage> capture_frame;

            {
              std::lock_guard capture_lock(mutexCaptureQueue);
              capture_frame = captureFrameQueue.front();
              captureFrameQueue.pop();
            }

            if (capture_frame)
            {
              const auto surface = capture_frame->GetImage(0);
              const auto image_data = static_cast<uint8_t *>(surface->pixels);
              const auto surface_data_size = static_cast<uint32_t>(surface->h * surface->pitch);

              hr = buffer->Lock(&buffer_data, &max_length, &current_length);
              if (FAILED(hr))
              {
                spdlog::warn("Failed to lock buffer");
                buffer.Reset();
                failCount++;
                captureFrameQueue.push(capture_frame);
                continue;
              }

              if (current_length > 0 && (surface_data_size > current_length || ignoreFailFormat))
              {
                const int32_t copy_byte_size = (surface_data_size > current_length) ? current_length : surface_data_size;
                utility::fast_memcpy(image_data, buffer_data, copy_byte_size);
              }
              else
              {
                spdlog::warn("Failed to copy buffer");

                hr = buffer->Unlock();
                if (FAILED(hr))
                {
                  spdlog::warn("Failed to unlock buffer");
                }

                captureFrameQueue.push(capture_frame);

                continue;
              }

              {
                std::lock_guard ready_lock(mutexReadyQueue);
                frameCount++;

                auto q_ready_frame = std::shared_ptr<FImage>(capture_frame.get(), [q_frame = capture_frame, &capture_queue = this->captureFrameQueue, &mtx_capture = this->mutexCaptureQueue]([[maybe_unused]] FImage* frame) {
                  std::lock_guard capture_lock(mtx_capture);
                  if (q_frame)
                  {
                    capture_queue.push(q_frame);
                  }
                });
                readyFrameQueue.push(q_ready_frame);
              }
            }
            else
            {
              skipFrames++;
            }
          }
        }
      }

      if (failCount > max_capture_fails)
      {
        isCapturing = false;
      }
    }

    {
      std::lock_guard lock_ready_queue(mutexReadyQueue);
      while (!readyFrameQueue.empty())
      {
        readyFrameQueue.pop();
      }
    }

    while (captureFrameQueue.size() < frames.size())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100u));
    }

    {
      std::lock_guard lock_capture_queue(mutexCaptureQueue);
      while (!captureFrameQueue.empty())
      {
        captureFrameQueue.pop();
      }
    }

    isInitialized = false;
    frameCount = 0;

    spdlog::info("Finished capturing");
    std::this_thread::sleep_for(std::chrono::milliseconds(100u));
  }

  void WinCamera::SampleFramesThread()
  {
    while (isCapturing)
    {
      DWORD stream_index;
      DWORD stream_flags;
      LONGLONG timestamp;
      Microsoft::WRL::ComPtr<IMFSample> sample;

      const HRESULT hr = reader->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM)
                                           ,0
                                           ,&stream_index
                                           ,&stream_flags
                                           ,&timestamp
                                           ,&sample);

      if (FAILED(hr))
      {
        spdlog::warn("Unable to query video frame sample");
      }

      if (sample && SUCCEEDED(hr))
      {
        if (!isPause)
        {
          std::lock_guard lock_sample_queue(mutexQueue);
          sampleBufferQueue.push({sample, stream_index, stream_flags, timestamp});
        }
        else
        {
          sample.Reset();
        }
      }
    }
  }

  int32_t WinCamera::FindCaptureDeviceByPath(const std::string_view path)
  {
    int32_t index = -1;

    IMFActivate** m_devices = nullptr;
    IMFAttributes* m_attributes = nullptr;
    uint32_t num_devices = 0;

    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
    {
      spdlog::error("Failed to initialize MFStartup");
      return -1;
    }

    hr = MFCreateAttributes(&m_attributes, 1);
    if (FAILED(hr))
    {
      spdlog::error("Failed to create attributes");
      return -1;
    }

    hr = m_attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr))
    {
      spdlog::error("Failed to set VIDCAP_GUID");
      return -1;
    }

    hr = MFEnumDeviceSources(m_attributes, &m_devices, &num_devices);
    if (FAILED(hr) || num_devices == 0)
    {
      spdlog::error("Failed to enumerate devices sources");
      return -1;
    }

    for (uint32_t i = 0; i < num_devices; i++)
    {
      WCHAR* w_friendly_name = nullptr;
      WCHAR* w_device_path = nullptr;
      uint32_t device_path_length = 0;
      std::string info_friendly_name;
      std::string info_device_path;

      const HRESULT hr_friendly = m_devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &w_friendly_name, &device_path_length);
      if (SUCCEEDED(hr_friendly))
      {
        if (const int32_t m_buffer_size = WideCharToMultiByte(CP_UTF8, 0, w_friendly_name, -1, nullptr, 0, nullptr, nullptr))
        {
          char* m_buffer = new char[m_buffer_size];
          WideCharToMultiByte(CP_UTF8, 0, w_friendly_name, -1, m_buffer, m_buffer_size, nullptr, nullptr);
          info_friendly_name = std::string(m_buffer);
          delete[] m_buffer;
        }
      }

      const HRESULT hr_device_path = m_devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &w_device_path, &device_path_length);
      if (SUCCEEDED(hr_device_path))
      {
        if (const int32_t m_buffer_size = WideCharToMultiByte(CP_UTF8, 0, w_device_path, -1, nullptr, 0, nullptr, nullptr))
        {
          const auto m_buffer = new char[m_buffer_size];
          WideCharToMultiByte(CP_UTF8, 0, w_device_path, -1, m_buffer, m_buffer_size, nullptr, nullptr);
          info_device_path = std::string(m_buffer);

          if (info_device_path == path)
          {
            index = i;
          }

          delete[] m_buffer;
        }
      }

      if (SUCCEEDED(hr_friendly) && SUCCEEDED(hr_device_path))
      {
        spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::bright_cyan), "Device Name: {} | Device Path: {}", info_friendly_name, utility::escape_control_chars(info_device_path)));
      }

    }

    if (m_attributes)
    {
      m_attributes->Release();
      m_attributes = nullptr;
    }

    if (m_devices && num_devices > 0)
    {
      for (size_t i=0; i<num_devices; i++)
      {
        m_devices[i]->Release();
      }
    }

    if (m_devices)
    {
      CoTaskMemFree(devices);
      devices = nullptr;
    }

    hr = MFShutdown();
    if (FAILED(hr))
    {
      spdlog::error("Failed to shutdown Media Foundation For Video Capture");
    }

    return index;
  }

  std::string WinCamera::FindCaptureDeviceByPath(const int32_t capture_device_index)
  {
    std::string device_path = "null";

    IMFActivate** m_devices = nullptr;
    IMFAttributes* m_attributes = nullptr;
    uint32_t num_devices = 0;

    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
    {
      spdlog::error("Failed to initialize MFStartup");
      return "";
    }

    hr = MFCreateAttributes(&m_attributes, 1);
    if (FAILED(hr))
    {
      spdlog::error("Failed to create attributes");
      return "";
    }

    hr = m_attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr))
    {
      spdlog::error("Failed to set VIDCAP_GUID");
      return "";
    }

    hr = MFEnumDeviceSources(m_attributes, &m_devices, &num_devices);
    if (FAILED(hr) || num_devices == 0)
    {
      spdlog::error("Failed to enumerate devices sources");
      return "";
    }

    for (uint32_t i = 0; i < num_devices; i++)
    {
      WCHAR* w_friendly_name = nullptr;
      WCHAR* w_device_path = nullptr;
      uint32_t device_path_length = 0;
      std::string info_friendly_name;
      std::string info_device_path;

      const HRESULT hr_friendly = m_devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &w_friendly_name, &device_path_length);
      if (SUCCEEDED(hr_friendly))
      {
        if (const int32_t m_buffer_size = WideCharToMultiByte(CP_UTF8, 0, w_friendly_name, -1, nullptr, 0, nullptr, nullptr))
        {
          char* m_buffer = new char[m_buffer_size];
          WideCharToMultiByte(CP_UTF8, 0, w_friendly_name, -1, m_buffer, m_buffer_size, nullptr, nullptr);
          info_friendly_name = std::string(m_buffer);
          delete[] m_buffer;
        }
      }

      const HRESULT hr_device_path = m_devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &w_device_path, &device_path_length);
      if (SUCCEEDED(hr_device_path))
      {
        if (const int32_t m_buffer_size = WideCharToMultiByte(CP_UTF8, 0, w_device_path, -1, nullptr, 0, nullptr, nullptr))
        {
          const auto m_buffer = new char[m_buffer_size];
          WideCharToMultiByte(CP_UTF8, 0, w_device_path, -1, m_buffer, m_buffer_size, nullptr, nullptr);
          info_device_path = std::string(m_buffer);

          if (capture_device_index == static_cast<int32_t>(i))
          {
            device_path = info_device_path;
          }

          delete[] m_buffer;
        }
      }

      if (SUCCEEDED(hr_friendly) && SUCCEEDED(hr_device_path))
      {
        spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::bright_cyan), "Device Name: {} | Device Path: {}", info_friendly_name, utility::escape_control_chars(info_device_path)));
      }
    }

    if (m_attributes)
    {
      m_attributes->Release();
      m_attributes = nullptr;
    }

    if (m_devices && num_devices > 0)
    {
      for (size_t i=0; i<num_devices; i++)
      {
        m_devices[i]->Release();
      }
    }

    if (m_devices)
    {
      CoTaskMemFree(devices);
      devices = nullptr;
    }

    hr = MFShutdown();
    if (FAILED(hr))
    {
      spdlog::error("Failed to shutdown Media Foundation For Video Capture");
    }

    return device_path;
  }
}