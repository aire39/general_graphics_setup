#include "RosVideoFeed.h"

#include <iostream>
#include "graphics/images/FImage.h"
#include <sensor_msgs/msg/image.hpp>

RosVideoFeed::RosVideoFeed()
  : rclcpp::Node("image_subscriber")
{
  image_sub = this->create_subscription<sensor_msgs::msg::Image>(
    "/camera/camera/image_raw"
   ,rclcpp::SensorDataQoS()
   ,std::bind(&RosVideoFeed::image_callback, this, std::placeholders::_1)
);
}

RosVideoFeed::~RosVideoFeed()
{

}

void RosVideoFeed::StartCapture()
{
  isCapturing = true;
}

void RosVideoFeed::PauseCapture()
{
  isCapturing = false;
}

void RosVideoFeed::StopCapture()
{
  isCapturing = false;
}

void RosVideoFeed::Reset()
{

}

void RosVideoFeed::ChangeFramerate([[maybe_unused]] gss::video::camera::types::FrameRate frame_rate, [[maybe_unused]] bool reset)
{

}

void RosVideoFeed::ChangeResolution([[maybe_unused]] gss::video::camera::types::FrameSize frame_size, [[maybe_unused]] bool reset)
{

}

void RosVideoFeed::ChangePixelFormat([[maybe_unused]] gss::video::camera::types::VideoFormat video_format, [[maybe_unused]] bool reset)
{

}

void RosVideoFeed::IgnoreFormatFail([[maybe_unused]] bool ignore, [[maybe_unused]] bool reset)
{

}

std::shared_ptr<FImage> RosVideoFeed::ExtractFrame()
{
  return nullptr;
}

std::shared_ptr<FImage> RosVideoFeed::CopyFrame()
{
  return nullptr;
}

gss::video::camera::types::FrameRate RosVideoFeed::GetFramerate() const
{
  return {1, 30};
}

gss::video::camera::types::FrameSize RosVideoFeed::GetResolution() const
{
  return {640, 480};
}

gss::video::camera::types::VideoFormat RosVideoFeed::GetPixelFormat() const
{
  return 0;
}

float RosVideoFeed::GetRunningFps() const
{
  return 0;
}

bool RosVideoFeed::IsCapturing() const
{
  return false;
}

uint32_t RosVideoFeed::GetFailCount() const
{
  return 0;
}

uint32_t RosVideoFeed::GetSkippedFrames() const
{
  return 0;
}

uint32_t RosVideoFeed::GetMaxBufers() const
{
  return 0;
}

uint32_t RosVideoFeed::GetFrameCount() const
{
  return 0;
}

bool RosVideoFeed::AllocateBuffers()
{
  return true;
}

bool RosVideoFeed::Configuration()
{
  return true;
}

void RosVideoFeed::CaptureFramesThread()
{
}

void RosVideoFeed::image_callback([[maybe_unused]] const sensor_msgs::msg::Image::SharedPtr msg)
{
  std::cout << "received image data!" << std::endl;
  //cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, msg->encoding);
  //cv::Mat img = cv_ptr->image;
}
