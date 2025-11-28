#include "OpticalFlowBlock.h"

#include <ranges>
#include <SDL3/SDL.h>
#include "graphics/images/FImage.h"
#include "CornerKeyPointsData.h"
#include "support/logging.h"

namespace {
  constexpr float lpf_smooth_factor = 0.1f;

  cv::Mat ConvertToSingleChannel(std::shared_ptr<FImage>& image)
  {
    cv::Mat rgb_image(image->GetHeight(), image->GetWidth(), CV_8UC3, image->GetImage(0)->pixels);
    cv::Mat gray;
    cv::cvtColor(rgb_image, gray, cv::COLOR_RGB2GRAY);
    gray.convertTo(gray, CV_32F, 1.0 / 255.0);

    return gray;
  }

  cv::Mat ConvertGrayToSingleChannel(std::shared_ptr<FImage>& image)
  {
    cv::Mat rgb_image(image->GetHeight(), image->GetWidth(), CV_8UC3, image->GetImage(0)->pixels);
    cv::Mat gray;
    cv::cvtColor(rgb_image, gray, cv::COLOR_RGB2GRAY);
    gray.convertTo(gray, CV_32F, 1.0 / 255.0);

    return gray;
  }
}

OpticalFlowBlock::OpticalFlowBlock(const std::string &thread_name, const std::string &thread_description)
{
  name = thread_name;
  description = thread_description;
  dataFlow = DataFlow::F_INOUT;
}

OpticalFlowBlock::OpticalFlowBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others)
{
  name = thread_name;
  description = thread_description;
  connections = others;
  dataFlow = DataFlow::F_INOUT;
}

void OpticalFlowBlock::SetMaxFlowSteps(size_t max_flow_steps)
{
  maxflowSteps = max_flow_steps;
}

void OpticalFlowBlock::SetMinPoints(size_t min_points)
{
  trackPointMinThreshold = min_points;
}

uint32_t OpticalFlowBlock::GetNumberOfGoodTracks() const
{
  return static_cast<uint32_t>(goodTrackPoints.size());
}

std::vector<std::shared_ptr<FImage>> OpticalFlowBlock::Process(std::vector<std::shared_ptr<FImage>> image_sources, DataContainer& data_sources)
{
  std::shared_ptr<FImage> image_source = nullptr;
  if (!image_sources.empty())
  {
    image_source = image_sources.back();
  }

  const auto start_time_process = std::chrono::high_resolution_clock::now();

  std::vector<cv::KeyPoint> good_track_points;

  if (lastGradXFimage)
  {
    cv::Mat last_grad_x = ConvertToSingleChannel(lastGradXFimage);
    cv::Mat last_grad_y = ConvertToSingleChannel(lastGradYFimage);
    cv::Mat last_gray = ConvertGrayToSingleChannel(lastGrayImage);

    cv::Mat current_grad_x = ConvertToSingleChannel(image_sources[0]);
    cv::Mat current_grad_y = ConvertToSingleChannel(image_sources[1]);
    cv::Mat current_gray = ConvertGrayToSingleChannel(image_sources[2]);

    int32_t window_size = 7;
    int32_t half_window_size = window_size / 2;

    for (auto point : lastTrackPoints)
    {
      int32_t x = static_cast<int32_t>(point.pt.x);
      int32_t y = static_cast<int32_t>(point.pt.y);

      if (x < half_window_size || y < half_window_size || x >= (last_gray.cols - half_window_size) || y >= (last_gray.rows - half_window_size))
      {
        continue;
      }

      float u = 0.0f;
      float v = 0.0f;

      for (size_t flow_step=0; flow_step < maxflowSteps; flow_step++)
      {
        float gxx = 0.0f;
        float gyy = 0.0f;
        float gxy = 0.0f;
        float bx = 0.0f;
        float by = 0.0f;

        for (int32_t i = -half_window_size; i <= half_window_size; i++)
        {
          for (int32_t j = -half_window_size; j <= half_window_size; j++)
          {
            float ix = last_grad_x.at<float>(y + i, x + j);
            float iy = last_grad_y.at<float>(y + i, x + j);
            float it = last_gray.at<float>(y + i, x + j) - current_gray.at<float>(y + i, x + j);

            gxx += ix * ix;
            gyy += iy * iy;
            gxy += ix * iy;
            bx += ix * it;
            by += iy * it;
          }
        }

        float det = (gxx * gyy) - (gxy * gxy);
        if (std::fabs(det) < 1e-6)
        {
          continue;
        }

        float inv_det = 1.0f / det;
        float du = ((-gyy * bx) + (gxy * by)) * inv_det;
        float dv = ((-gxy * bx) + (gxx * by)) * inv_det;

        u += du;
        v += dv;

        // Stop early if the motion converged
        if (std::sqrt(du * du + dv * dv) < 0.001f)
        {
          break;
        }
      }

      good_track_points.push_back(cv::KeyPoint{(point.pt.x + u), (point.pt.y + v), 1.0f});
    }

  }
  else
  {
    lastGradXFimage = image_sources[0];
    lastGradYFimage = image_sources[1];
    lastGrayImage   = image_sources[2];
  }

  if (!good_track_points.empty())
  {
    goodTrackPoints = good_track_points;
  }

  const auto keypoints = data_sources.GetData<CornerKeyPointsData>();
  if (keypoints->keypoints.size() > trackPointMinThreshold && (lastTrackPoints.empty() || lastTrackPoints.size() < trackPointMinThreshold))
  {
    lastTrackPoints = keypoints->keypoints;
  }
  else
  {
    if (!good_track_points.empty())
    {
      lastGoodTrackPoints = lastTrackPoints;
      lastTrackPoints = good_track_points;
    }
  }

  lastGradXFimage = image_sources[0];
  lastGradYFimage = image_sources[1];
  lastGrayImage = image_sources[2];

  cv::Mat features_map(image_source->GetHeight(), image_source->GetWidth(), CV_8UC3, cv::Scalar(0, 0, 0));

  for (auto&& [tp0, tp1] : std::views::zip(good_track_points, lastGoodTrackPoints))
  {
    cv::circle(features_map, tp0.pt, 3, cv::Scalar(255.0, 255.0, 255.0), 1, cv::LINE_AA);

    auto flow = (tp1.pt - tp0.pt) * 2.5f;
    cv::line(features_map, tp1.pt, tp0.pt + flow, cv::Scalar(0.0, 255.0, 0.0), 1, cv::LINE_AA);
  }

  cv::Mat rgb_image(image_sources[2]->GetHeight(), image_sources[2]->GetWidth(), CV_8UC3, image_sources[2]->GetImage(0)->pixels);
  features_map = rgb_image + features_map;

  auto image_result = std::make_shared<FImage>("final_corners", features_map.cols, features_map.rows, SDL_PixelFormat::SDL_PIXELFORMAT_RGB24, true);
  std::memcpy(image_result->GetImage(0)->pixels, features_map.data, static_cast<size_t>(image_result->GetHeight() * image_result->GetWidth() * 3));

  const auto end_time_process = std::chrono::high_resolution_clock::now();
  const int64_t tick_count = std::chrono::duration_cast<std::chrono::microseconds>(end_time_process - start_time_process).count();
  timeToComplete = static_cast<float>(tick_count) / 1000.0f;
  timeFilterToComplete = (lpf_smooth_factor * timeToComplete) + (1.0f - lpf_smooth_factor) * timeFilterToComplete;

  return {image_result};
}
