#include "LinuxCamera.h"
#include "graphics/FImage.h"

#include "SDL3/SDL_surface.h"
#include "common/support/logging.h"
#include <spdlog/fmt/bundled/color.h>

#include "common/support/utility.h"
#include "video/camera/common/CameraFormats.h"
#include "graphics/Filters.h"

#if ENABLE_LINUX_PLATFORM_CHECKS && defined(__linux__) || defined(__unix__)
#include "video/camera/common/CameraChecks.h"
#endif

#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

namespace {
  constexpr std::string_view base_video_path = "/dev/video";
  constexpr uint32_t max_capture_fails = 15;
  constexpr uint32_t default_device_index = 0;
  constexpr uint32_t default_frame_width = 640;
  constexpr uint32_t default_frame_height = 480;
  constexpr uint32_t default_number_of_buffers = 8;
  constexpr uint32_t default_frame_timeout_ms = 500;
  constexpr uint32_t default_no_active_count = 10;
  constexpr bool default_ignore_fail_format = false;
  constexpr double lpf_smooth_factor = 0.1;
  constexpr gss::video::camera::types::FrameRate default_frame_fps = {30, 1};
  constexpr gss::video::camera::types::VideoFormat default_format = gss::video::camera::formats::UYVY_FORMAT;

  bool xioctl(const int32_t& fd, const uint32_t request, void* data)
  {
    bool success = true;

    while (const auto ret = ioctl(fd, request, data))
    {
      if (ret == -1 && (errno == EAGAIN || errno == EINTR))
      {
        continue;
      }
      else
      {
        success = false;
        break;
      }
    }

    return success;
  }
}

namespace gss::video::camera::platform {
  LinuxCamera::LinuxCamera()
    : LinuxCamera(default_device_index, default_frame_width, default_frame_height, default_frame_fps, default_format, default_ignore_fail_format)
  {

  }

  LinuxCamera::LinuxCamera(const uint32_t capture_device_index, const int32_t width, const int32_t height, const gss::video::camera::types::FrameRate &fps, const gss::video::camera::types::VideoFormat format, bool ignore_fail_format)
    : LinuxCamera(std::string(base_video_path) + std::to_string(capture_device_index), width, height, fps, format, ignore_fail_format)
  {
  }

  LinuxCamera::LinuxCamera(const std::string &device_path, const int32_t width, const int32_t height, const gss::video::camera::types::FrameRate &fps, const gss::video::camera::types::VideoFormat format, const bool ignore_fail_format)
    :width(width)
    ,s_width(width)
    ,height(height)
    ,s_height(height)
    ,fps(fps)
    ,s_fps(fps)
    ,ignoreFailFormat(ignore_fail_format)
    ,s_ignoreFailFormat(ignoreFailFormat)
    ,devicePath(device_path)
    ,videoFormat(format)
    ,s_videoFormat(videoFormat)
  {
    const std::string_view sv_device_path = device_path;
    const std::string_view sv_device_name = sv_device_path.substr(sv_device_path.rfind('/') + 1);
    captureDevice = utility::parse_number_from_suffix(sv_device_name);

    fd = open(device_path.c_str(), O_RDWR | O_NONBLOCK);
    logging::info("Opened device: {} ({})", device_path, fd);
  }

  LinuxCamera::~LinuxCamera()
  {
    StopCapture();
  }

  void LinuxCamera::StartCapture()
  {
    width = s_width;
    height = s_height;
    fps = s_fps;
    videoFormat = s_videoFormat;
    ignoreFailFormat = s_ignoreFailFormat;

    if ((fd < 0) && !isCapturing)
    {
      isPause = false;
      fd = open(devicePath.c_str(), O_RDWR | O_NONBLOCK);
      logging::info("Opened device: {} ({})", devicePath, fd);
    }

    if (fd >= 0)
    {
      if (isPause && isCapturing)
      {
        isPause = false;
      }
      else
      {
        if (AllocateBuffers())
        {
          if(((isInitialized = LinuxCamera::Configuration())) || ignoreFailFormat)
          {
            isCapturing = true;

            v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (!xioctl(fd, VIDIOC_STREAMON, &type))
            {
              logging::error("VIDIOC_STREAMON failed: {}", strerror(errno));
              return;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100u));

            captureThread = cthread("cap-camera", "capture frame data", &LinuxCamera::CaptureFramesThread, this);
          }
          else
          {
            logging::warn("Video device is not initialized");
          }
        }
        else
        {
          logging::error("Failed to allocate user buffers");
        }
      }
    }
    else
    {
      logging::warn("Unable to open device");
    }
  }

  void LinuxCamera::PauseCapture()
  {
    isPause = true;
  }

  void LinuxCamera::StopCapture()
  {
    if (isInitialized && (fd >= 0))
    {
      PauseCapture();

      isInitialized = false;
      isCapturing = false;

      if (captureThread.joinable())
      {
        captureThread.join();
      }

      struct v4l2_buffer buf_dq {};
      buf_dq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf_dq.memory = V4L2_MEMORY_MMAP;

      while (!xioctl(fd, VIDIOC_DQBUF, &buf_dq))
      {
        auto e = errno;

        if (e != EAGAIN)
        {
          logging::warn("Unable to dequeue buffer for cleanup: ({}) {}", 0, strerror(e), e);
          break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10u));
      }

      v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if (!xioctl(fd, VIDIOC_STREAMOFF, &type))
      {
        logging::error("VIDIOC_STREAMOFF failed: {}", strerror(errno));
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(100u));

      for (size_t i = 0; i < buffers.size(); i++)
      {
        if (munmap(buffers[i].first, buffers[i].second) < 0)
        {
          logging::error("unable unmap buffer {}", i);
        }
      }


      struct v4l2_requestbuffers req = {};
      req.count = 0;
      req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      req.memory = V4L2_MEMORY_MMAP;
      if (!xioctl(fd, VIDIOC_REQBUFS, &req))
      {
        logging::warn("Failed to zero requested buffers");
      }

      buffers.clear();
      frames.clear();

      queuedBuffers = 0;
      failCount = 0;
      notActiveCount = 0;
      isPause = false;

      close(fd);
      fd = -1;

#if ENABLE_LINUX_PLATFORM_CHECKS && (defined(__linux__) || defined(__unix__))
      // this trick requires that the application be called with sudo or be part of the sudoers list
      // this is here due to an issue with the usb not resetting or letting go of resources properly
      // should work without this trick normally
      std::string_view device_path = devicePath;
      std::string_view device_name = device_path.substr(device_path.rfind('/')+1);
      gss::video::camera::checks::reset_camera_usb_from_path(std::string(device_name));
#endif
    }
  }

  void LinuxCamera::Reset()
  {
    StopCapture();
    StartCapture();
  }

  void LinuxCamera::ChangeFramerate(gss::video::camera::types::FrameRate frame_rate, bool reset)
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

  void LinuxCamera::ChangeResolution(gss::video::camera::types::FrameSize frame_size, bool reset)
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

  void LinuxCamera::ChangePixelFormat(gss::video::camera::types::VideoFormat video_format, bool reset)
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

  void LinuxCamera::IgnoreFormatFail(bool ignore, bool reset)
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

  std::shared_ptr<FImage> LinuxCamera::ExtractFrame()
  {
    std::lock_guard lock(mutexReadyQueue);

    std::shared_ptr<FImage> extract_frame = nullptr;

    if (!readyFrameQueue.empty())
    {
      extract_frame = readyFrameQueue.front();
      readyFrameQueue.pop();
    }

    return extract_frame;
  }

  std::shared_ptr<FImage> LinuxCamera::CopyFrame()
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

  gss::video::camera::types::FrameRate LinuxCamera::GetFramerate() const
  {
    return fps;
  }

  gss::video::camera::types::FrameSize LinuxCamera::GetResolution() const
  {
    return {width, height};
  }

  gss::video::camera::types::VideoFormat LinuxCamera::GetPixelFormat() const
  {
    return videoFormat;
  }

  float LinuxCamera::GetRunningFps() const
  {
    return 1.0f / static_cast<float>(runningFrameTime);
  }

  bool LinuxCamera::IsCapturing() const
  {
    return isCapturing;
  }

  uint32_t LinuxCamera::GetFailCount() const
  {
    return failCount;
  }

  uint32_t LinuxCamera::GetSkippedFrames() const
  {
    return skipFrames;
  }

  uint32_t LinuxCamera::GetMaxBufers() const
  {
    return default_number_of_buffers;
  }

  uint32_t LinuxCamera::GetFrameCount() const
  {
    return frameCount;
  }

  bool LinuxCamera::AllocateBuffers()
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

    if (fd > 0)
    {
      for (size_t i=0; i<default_number_of_buffers; i++)
      {
        constexpr bool base_same_as_orig = true;
        constexpr bool save_filter = true;
        std::string frame_name = "frame-" + std::to_string(i);

        auto new_fimage = std::make_shared<FImage>(frame_name, width, height, SDL_PixelFormat::SDL_PIXELFORMAT_RGB24, base_same_as_orig);
        new_fimage->ProcessFilter(use_filter, save_filter);
        frames.push_back(new_fimage);
      }
    }

    return true;
  }

  bool LinuxCamera::Configuration()
  {
    struct v4l2_format fmt {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (!xioctl(fd, VIDIOC_G_FMT, &fmt))
    {
      logging::warn("Failed to get capture format");
      return false;
    }


    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = videoFormat;


    if (!xioctl(fd, VIDIOC_S_FMT, &fmt))
    {
      logging::warn("Failed to set capture format");
      return false;
    }

    struct v4l2_streamparm streamparm {};
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(fd, VIDIOC_G_PARM, &streamparm);

    streamparm.parm.capture.capturemode |= V4L2_CAP_TIMEPERFRAME;
    streamparm.parm.capture.timeperframe.numerator = fps.first;
    streamparm.parm.capture.timeperframe.denominator = fps.second;

    if (!xioctl(fd, VIDIOC_S_PARM, &streamparm))
    {
      logging::warn("Failed to set video frame rate");
      return false;
    }

    struct v4l2_requestbuffers req {};
    req.count = default_number_of_buffers;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (!xioctl(fd, VIDIOC_REQBUFS, &req))
    {
      logging::warn("Failed to request buffers");
      return false;
    }

    logging::info("requested buffer size: {}", req.count);

    // initialize/store/queue buffers

    for (size_t i = 0; i < default_number_of_buffers; i++)
    {
      struct v4l2_buffer buf {};
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = i;

      // query buffers to get address

      if (!xioctl(fd, VIDIOC_QUERYBUF, &buf))
      {
        logging::warn("Failed to query buffer");
        return false;
      }

      auto map_buffer = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
      if (map_buffer == MAP_FAILED)
      {
        logging::warn("Failed to map buffer");
        return false;
      }

      // queue the buffers

      if (!xioctl(fd, VIDIOC_QBUF, &buf))
      {
        logging::warn("Failed to queue buffer");
        return false;
      }

      // store the buffers
      buffers.emplace_back(static_cast<uint8_t*>(map_buffer), buf.length);
      queuedBuffers++;
    }

    return true;
  }

  void LinuxCamera::CaptureFramesThread()
  {
    double filtered_timestamp = 0.0;
    double previous_filtered_timestamp = 0.0;

    while (isCapturing)
    {
      if (isPause)
      {
        while (!readyFrameQueue.empty())
        {
          readyFrameQueue.pop();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100u));
        continue;
      }

      if (failCount >= max_capture_fails)
      {
        isCapturing = false;
        continue;
      }

      struct v4l2_buffer buf_dq {};
      buf_dq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf_dq.memory = V4L2_MEMORY_MMAP;

      if (WaitForFrame())
      {
        if (!xioctl(fd, VIDIOC_DQBUF, &buf_dq))
        {
          int error_value = errno;
          if (error_value != EAGAIN)
          {
            failCount++;
            logging::warn("Failed to dequeue buffer ({})", strerror(error_value));
          }

          continue;
        }

        {
          const double current_timestamp = (buf_dq.timestamp.tv_sec * 1e9) + (buf_dq.timestamp.tv_usec * 1e3);

          if (filtered_timestamp != 0)
          {
            filtered_timestamp = (lpf_smooth_factor * current_timestamp) + (1.0 - lpf_smooth_factor) * filtered_timestamp;
            runningFrameTime = (filtered_timestamp - previous_filtered_timestamp) / 1e9;
            previous_filtered_timestamp = filtered_timestamp;
          }
          else
          {
            filtered_timestamp = current_timestamp;
            previous_filtered_timestamp = filtered_timestamp;
          }
        }

        logging::info("frame {} dequeued", buf_dq.index);
      }
      else
      {
        if (notActiveCount > default_no_active_count)
        {
          auto stop_capture_thread = std::thread([this](){StopCapture();});
          stop_capture_thread.detach();
          logging::warn("Failed trying to dequeue frames. Stopping camera!");
        }

        skipFrames++;

        continue;
      }

      {
        std::lock_guard lock(mutexQueueTrack);
        queuedBuffers--;
      }

      frameCount++;

      const uint8_t* buffer = buffers[buf_dq.index].first;

      logging::info("buffer index: {}", buf_dq.index);

      // copy data to FImage
      std::shared_ptr<FImage> frame(frames[buf_dq.index].get(), [this, buffer_index=buf_dq.index](FImage*) {
        QueueFrame(buffer_index);
      });

      const auto surface = frame->GetImage(0);
      const uint32_t surface_data_size = (surface->h * surface->pitch);

      if ((buf_dq.length > 0) && (surface_data_size > buf_dq.length))
      {
        const int32_t copy_byte_size = (surface_data_size > buf_dq.length) ? buf_dq.length : surface_data_size;
        utility::fast_memcpy(surface->pixels, buffer, copy_byte_size);
      }
      else
      {
        logging::warn("Failed to copy buffer");
        frame.reset(); // will queue buffer if this copy fails
        failCount++;
        continue;
      }

      failCount = 0;

      {
        std::lock_guard lock(mutexReadyQueue);
        readyFrameQueue.push(frame);
      }

      logging::info("frame: {}", frameCount);
    }

    {
      std::lock_guard lock(mutexReadyQueue);
      while(!readyFrameQueue.empty())
      {
        readyFrameQueue.pop();
        std::this_thread::sleep_for(std::chrono::milliseconds(10u));
      }
    }

    while (queuedBuffers < frames.size())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(16u));
    }
  }

  bool LinuxCamera::WaitForFrame()
  {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    struct timeval timeout {};
    timeout.tv_sec = default_frame_timeout_ms / 1000;
    timeout.tv_usec = (default_frame_timeout_ms - timeout.tv_sec * 1000) * 1000;

    const auto timeout_result = select(fd + 1, &read_fds, nullptr, nullptr, &timeout);

    if (timeout_result == 0)
    {
      notActiveCount++;
    }
    else
    {
      notActiveCount = 0;
    }

    return timeout_result > 0;
  }

  bool LinuxCamera::QueueFrame(int32_t buffer_index)
  {
    bool query_success = true;
    bool success = true;

    if (fd > 0)
    {
      struct v4l2_buffer buf {};
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = buffer_index;

      if (!xioctl(fd, VIDIOC_QUERYBUF, &buf))
      {
        logging::warn("Failed to query buffer: {} ({})", buf.index, strerror(errno));
        query_success = false;
      }

      if (query_success && !xioctl(fd, VIDIOC_QBUF, &buf))
      {
        logging::warn("Failed to enqueue buffer: {}({}) ({})", buf.index, buffer_index, strerror(errno));
        success = false;
      }

      if (success && query_success)
      {
        std::lock_guard lock(mutexQueueTrack);
        queuedBuffers++;
        logging::info("enqueued buffer ({})", queuedBuffers);
      }
    }

    return (success && query_success);
  }
}
