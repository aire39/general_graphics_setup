#include "PixelCornersBlock.h"

#include <chrono>

#include "graphics/images/FImage.h"
#include "examples/camera-streaming/filter-copy-stream-video/filters/EdgeFilters.h"

#include "opencv2/opencv.hpp"
#include "SDL3/SDL.h"

namespace {
  constexpr float lpf_smooth_factor = 0.1f;
}

PixelCornersBlock::PixelCornersBlock(const std::string &thread_name, const std::string &thread_description)
{
  name = thread_name;
  description = thread_description;
  dataFlow = DataFlow::F_INOUT;
}

PixelCornersBlock::PixelCornersBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others)
{
  name = thread_name;
  description = thread_description;
  connections = others;
  dataFlow = DataFlow::F_INOUT;
}

PixelCornersBlock::~PixelCornersBlock()
{
  Enable(false);
  if (produceGradXThread.joinable())
  {
    produceGradXThread.join();
  }

  if (produceGradYThread.joinable())
  {
    produceGradYThread.join();
  }
}

void PixelCornersBlock::SetSigma(const double sigma)
{
  sigmaFactor = sigma;
}

void PixelCornersBlock::SetKValue(const float k)
{
  kFactor = k;
}

float PixelCornersBlock::TimeToComplete() const
{
  return timeToComplete;
}

float PixelCornersBlock::TimeToCompleteFilter() const
{
  return timeFilterToComplete;
}

std::vector<std::shared_ptr<FImage>> PixelCornersBlock::Process(std::vector<std::shared_ptr<FImage>> image_sources)
{
  std::shared_ptr<FImage> image_source = nullptr;
  if (!image_sources.empty())
  {
    image_source = image_sources.back();
  }

  auto start_time_process = std::chrono::high_resolution_clock::now();

  gradXComplete = false;
  gradYComplete = false;

  cv::Mat input_rgb(image_source->GetHeight(), image_source->GetWidth(), CV_8UC3, image_source->GetImage(0)->pixels);

  cv::Mat gray;
  cv::cvtColor(input_rgb, gray, cv::COLOR_RGB2GRAY);
  gray.convertTo(gray, CV_32F, 1.0 / 255.0);

  // process gradient images

  cv::Mat grad_x;
  cv::Mat grad_y;
  sourcesGXQueue.push({std::ref(gray), std::ref(grad_x)});
  sourcesGYQueue.push({std::ref(gray), std::ref(grad_y)});

  receivedInputCondition.notify_all();

  {
    std::unique_lock lock(mtxGradX);
    produceGradXCondition.wait(lock, [this]() -> bool { return gradXComplete; });
  }

  {
    std::unique_lock lock(mtxGradY);
    produceGradYCondition.wait(lock, [this]() -> bool { return gradYComplete; });
  }

  // process special gradients

  cv::Mat grad_xy = grad_x.mul(grad_y);
  cv::Mat grad_xx = grad_x.mul(grad_x);
  cv::Mat grad_yy = grad_y.mul(grad_y);

  constexpr int32_t blockSize = 3;
  cv::GaussianBlur(grad_xx, grad_xx, cv::Size(blockSize, blockSize), sigmaFactor);
  cv::GaussianBlur(grad_yy, grad_yy, cv::Size(blockSize, blockSize), sigmaFactor);
  cv::GaussianBlur(grad_xy, grad_xy, cv::Size(blockSize, blockSize), sigmaFactor);

  cv::Mat detM = grad_xx.mul(grad_yy) - grad_xy.mul(grad_xy);
  cv::Mat traceM = grad_xx + grad_yy;
  cv::Mat R = detM - kFactor * traceM.mul(traceM);

  // process results

  cv::Mat gray3;
  cv::cvtColor(R, gray3, cv::COLOR_GRAY2RGB);
  gray3.convertTo(gray3, CV_8UC3, 255.0);

  auto image_result = std::make_shared<FImage>("corners", R.cols, R.rows, SDL_PixelFormat::SDL_PIXELFORMAT_RGB24, true);
  std::memcpy(image_result->GetImage(0)->pixels, gray3.data, image_result->GetHeight() * image_result->GetWidth() * 3);

  auto end_time_process = std::chrono::high_resolution_clock::now();

  int64_t tick_count = std::chrono::duration_cast<std::chrono::microseconds>(end_time_process - start_time_process).count();
  timeToComplete = static_cast<float>(tick_count) / 1000.0f;
  timeFilterToComplete = (lpf_smooth_factor * timeToComplete) + (1.0f - lpf_smooth_factor) * timeFilterToComplete;

  /*
  double minVal, maxVal;
  cv::minMaxLoc(grad_x, &minVal, &maxVal);
  logging::info("grad_x type: {} -- range: [{}, {}]", grad_x.type(), minVal, maxVal);

  cv::minMaxLoc(grad_y, &minVal, &maxVal);
  logging::info("grad_y type: {} -- range: [{}, {}]", grad_y.type(), minVal, maxVal);
  */

  return {image_result};
}

void PixelCornersBlock::ExtraEnableProcess()
{
  produceGradXThread = cthread("prod_gx", "produce gradient x", &PixelCornersBlock::GradXTask, this);
  produceGradYThread = cthread("prod_gy", "produce gradient y", &PixelCornersBlock::GradYTask, this);
}

void PixelCornersBlock::GradXTask()
{
  while (IsEnabled())
  {
    {
      std::unique_lock lock(mtxReceivedInput);
      receivedInputCondition.wait_for(lock, std::chrono::milliseconds(100u));
    }

    if (!sourcesGXQueue.empty())
    {
      auto & [gray_wrapper, grad_x_wrapper] = sourcesGXQueue.front();
      sourcesGXQueue.pop();

      auto& gray = gray_wrapper.get();
      auto& grad_x = grad_x_wrapper.get();

      cv::Scharr(gray, grad_x, CV_32F, 1, 0);

      {
        std::unique_lock lock(mtxGradX);
        gradXComplete = true;
      }

      produceGradXCondition.notify_one();
    }
  }
}

void PixelCornersBlock::GradYTask()
{
  while (IsEnabled())
  {
    {
      std::unique_lock lock(mtxReceivedInput);
      receivedInputCondition.wait_for(lock, std::chrono::milliseconds(100u));
    }

    if (!sourcesGYQueue.empty())
    {
      auto [gray_wrapper, grad_y_wrapper] = sourcesGYQueue.front();
      sourcesGYQueue.pop();

      auto& gray = gray_wrapper.get();
      auto& grad_y = grad_y_wrapper.get();

      cv::Scharr(gray, grad_y, CV_32F, 0, 1);

      {
        std::unique_lock lock(mtxGradY);
        gradYComplete = true;
      }

      produceGradYCondition.notify_one();
    }
  }
}
