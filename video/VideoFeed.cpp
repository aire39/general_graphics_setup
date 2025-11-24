#include "VideoFeed.h"

VideoFeed::VideoFeed()
{

}

VideoFeed::~VideoFeed()
{

}

void VideoFeed::StartCapture()
{

}

void VideoFeed::PauseCapture()
{

}

void VideoFeed::StopCapture()
{

}

void VideoFeed::Reset()
{

}

void VideoFeed::ChangeFramerate(gss::video::camera::types::FrameRate frame_rate, bool reset)
{

}

void VideoFeed::ChangeResolution(gss::video::camera::types::FrameSize frame_size, bool reset)
{

}

void VideoFeed::ChangePixelFormat(gss::video::camera::types::VideoFormat video_format, bool reset)
{

}

void VideoFeed::IgnoreFormatFail(bool ignore, bool reset)
{

}

std::shared_ptr<FImage> VideoFeed::ExtractFrame()
{
  return nullptr;
}

std::shared_ptr<FImage> VideoFeed::CopyFrame()
{
  return nullptr;
}

gss::video::camera::types::FrameRate VideoFeed::GetFramerate() const
{
  return 0;
}

gss::video::camera::types::FrameSize VideoFeed::GetResolution() const
{
  return 0;
}

gss::video::camera::types::VideoFormat VideoFeed::GetPixelFormat() const
{
  return 0;
}

float VideoFeed::GetRunningFps() const
{
  return 0;
}

bool VideoFeed::IsCapturing() const
{
  return false;
}

uint32_t VideoFeed::GetFailCount() const
{
  return 0;
}

uint32_t VideoFeed::GetSkippedFrames() const
{
  return 0;
}

uint32_t VideoFeed::GetMaxBufers() const
{
  return 0;
}

uint32_t VideoFeed::GetFrameCount() const
{
  return 0;
}

bool VideoFeed::AllocateBuffers()
{
  return 0;
}

bool VideoFeed::Configuration()
{
  return 0;
}

void VideoFeed::CaptureFramesThread()
{
  return 0;
}
