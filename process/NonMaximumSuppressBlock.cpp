#include "NonMaximumSuppressBlock.h"

#include <algorithm>
#include <chrono>
#include <cstring>

#include "common/support/cprocess.h"
#include "graphics/images/FImage.h"

#include "opencv2/opencv.hpp"
#include "SDL3/SDL.h"
#include "support/logging.h"

#include "CornerKeyPointsData.h"

namespace {
  constexpr float lpf_smooth_factor = 0.1f;
}

NonMaximumSuppressBlock::NonMaximumSuppressBlock()
  : NonMaximumSuppressBlock("nmsblock", "curtails corners")
{
}

NonMaximumSuppressBlock::NonMaximumSuppressBlock(const std::string &thread_name, const std::string &thread_description)
  : NonMaximumSuppressBlock(thread_name, thread_description, {})
{
}

NonMaximumSuppressBlock::NonMaximumSuppressBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others)
{
  name = thread_name;
  description = thread_description;
  connections = others;
  dataFlow = DataFlow::F_INOUT;
}

NonMaximumSuppressBlock::~NonMaximumSuppressBlock()
{
  Enable(false);
}

void NonMaximumSuppressBlock::SetResponseFactor(const double response_factor)
{
  responseThresholdFactor = response_factor;
}

void NonMaximumSuppressBlock::SetMinDistanceThreshold(const double min_distance)
{
  minDistanceThreshold = min_distance;
}

void NonMaximumSuppressBlock::SetPointSize(const double point_size)
{
  diameterThreshold = point_size;
}

void NonMaximumSuppressBlock::SetPointColor(std::array<double, 3> color)
{
  pointColor = color;
}

uint32_t NonMaximumSuppressBlock::GetNumberOfKeypoints() const
{
  return numberOfKeypoints;
}

std::vector<std::shared_ptr<FImage>> NonMaximumSuppressBlock::Process(std::vector<std::shared_ptr<FImage>> image_sources, [[maybe_unused]] DataContainer& data_sources)
{
  std::shared_ptr<FImage> image_source = nullptr;
  if (!image_sources.empty())
  {
    image_source = image_sources.back();
  }
  else
  {
    return {nullptr};
  }

  const auto start_time_process = std::chrono::high_resolution_clock::now();

  const cv::Mat input_response(image_source->GetHeight(), image_source->GetWidth(), CV_8UC3, image_source->GetImage(0)->pixels);

  cv::Mat response_mat;
  cv::cvtColor(input_response, response_mat, cv::COLOR_RGB2GRAY);
  response_mat.convertTo(response_mat, CV_32F, 1.0 / 255.0);

  // get max value of response

  double max_response_intensity = 0.0;
  cv::minMaxLoc(response_mat, nullptr, &max_response_intensity);
  const double response_threshold = responseThresholdFactor * max_response_intensity;

  // gather keypoints

  std::vector<cv::KeyPoint> key_points;

  for (int32_t y = 0; y<image_source->GetHeight(); y++)
  {
    for (int32_t x=0; x<image_source->GetWidth(); x++)
    {
      if (response_mat.at<float>(y, x) > response_threshold)
      {
        constexpr float point_angle = -1.0f;
        key_points.emplace_back(cv::Point2f(static_cast<float>(x), static_cast<float>(y)), static_cast<float>(diameterThreshold), point_angle, response_mat.at<float>(y, x));
      }
    }
  }

  // sort points by strength

  std::ranges::sort(key_points, [](const cv::KeyPoint& a, const cv::KeyPoint& b) -> bool {
    return a.response > b.response;
  });

  // suppress by distance

  finalKeyPoints.clear(); // potentially keep track of points and pass to the next step but this needs to be cleared
  for (const auto& kp : key_points)
  {
    bool too_close = false;
    for (const auto& fp : finalKeyPoints)
    {
      if (cv::norm(kp.pt - fp.pt) < minDistanceThreshold)
      {
        too_close = true;
        break;
      }
    }

    if (!too_close)
    {
      finalKeyPoints.push_back(kp);
    }
  }

  const auto keypoint_container = std::make_shared<CornerKeyPointsData>();
  keypoint_container->keypoints = finalKeyPoints;
  data_sources.Add(keypoint_container);

  numberOfKeypoints = static_cast<uint32_t>(finalKeyPoints.size());

  // render points

  cv::Mat features_map(image_source->GetHeight(), image_source->GetWidth(), CV_8UC3, cv::Scalar(0, 0, 0));

  for (auto& fp : finalKeyPoints)
  {
    cv::circle(features_map, fp.pt, static_cast<int32_t>(fp.size), cv::Scalar(pointColor[0] * 255.0, pointColor[1] * 255.0, pointColor[2] * 255.0), 1, cv::LINE_AA);
  }

  // copy result of image to be sent out

  auto image_result = std::make_shared<FImage>("final_corners", features_map.cols, features_map.rows, SDL_PixelFormat::SDL_PIXELFORMAT_RGB24, true);
  std::memcpy(image_result->GetImage(0)->pixels, features_map.data, static_cast<size_t>(image_result->GetHeight() * image_result->GetWidth() * 3));

  // timing information

  const auto end_time_process = std::chrono::high_resolution_clock::now();
  const int64_t tick_count = std::chrono::duration_cast<std::chrono::microseconds>(end_time_process - start_time_process).count();
  timeToComplete = static_cast<float>(tick_count) / 1000.0f;
  timeFilterToComplete = (lpf_smooth_factor * timeToComplete) + (1.0f - lpf_smooth_factor) * timeFilterToComplete;

  return {image_sources[0], image_sources[1], image_sources[2], image_result};
}

void NonMaximumSuppressBlock::ExtraEnableProcess()
{
}

void NonMaximumSuppressBlock::ExtraDisableProcess()
{
}
