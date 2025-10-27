#include "PixelCornersBlock.h"

#include <chrono>

#include "graphics/images/FImage.h"
#include "examples/camera-streaming/filter-copy-stream-video/filters/EdgeFilters.h"

#include "opencv2/opencv.hpp"
#include "SDL3/SDL.h"

namespace {
  constexpr float lpf_smooth_factor = 0.1f;
}

PixelCornersBlock::PixelCornersBlock()
  : PixelCornersBlock("pixcornerblock", "generates corners")
{
}

PixelCornersBlock::PixelCornersBlock(const std::string &thread_name, const std::string &thread_description)
  : PixelCornersBlock(thread_name, thread_description, {})
{
}

PixelCornersBlock::PixelCornersBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others)
  : gradSync(2, on_completion_grad(this))
  , gradBlurSync(3, on_completion_grad_blur(this))
{
  name = thread_name;
  description = thread_description;
  connections = others;
  dataFlow = DataFlow::F_INOUT;
}

PixelCornersBlock::~PixelCornersBlock()
{
  Enable(false);
}

void PixelCornersBlock::SetSigma(const double sigma)
{
  sigmaFactor = sigma;
}

void PixelCornersBlock::SetKValue(const float k)
{
  kFactor = k;
}

std::vector<std::shared_ptr<FImage>> PixelCornersBlock::Process(std::vector<std::shared_ptr<FImage>> image_sources)
{
  std::shared_ptr<FImage> image_source = nullptr;
  if (!image_sources.empty())
  {
    image_source = image_sources.back();
  }

  auto start_time_process = std::chrono::high_resolution_clock::now();

  gradComplete = false;
  gradBlurComplete = false;

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
    std::unique_lock lock(mtxGrad);
    produceGradCondition.wait_for(lock, std::chrono::milliseconds(1000u), [&](){ return gradComplete;} );
  }

  // process special gradients

  cv::Mat grad_xy;
  cv::Mat grad_xx;
  cv::Mat grad_yy;
  sourcesGXYQueue.push({std::ref(grad_x), std::ref(grad_y), std::ref(grad_xy)});
  sourcesGXXQueue.push({std::ref(grad_x), std::ref(grad_x), std::ref(grad_xx)});
  sourcesGYYQueue.push({std::ref(grad_y), std::ref(grad_y), std::ref(grad_yy)});

  receivedGradCondition.notify_all();

  {
    std::unique_lock lock(mtxGradBlur);
    produceGradBlurCondition.wait_for(lock, std::chrono::milliseconds(1000u), [&](){ return gradBlurComplete;} );
  }

  cv::Mat detM = grad_xx.mul(grad_yy) - grad_xy.mul(grad_xy);
  cv::Mat traceM = grad_xx + grad_yy;
  cv::Mat R = detM - kFactor * traceM.mul(traceM);

  // process results

  cv::Mat R3C;
  cv::cvtColor(R, R3C, cv::COLOR_GRAY2RGB);
  R3C.convertTo(R3C, CV_8UC3, 255.0);

  auto image_result = std::make_shared<FImage>("corners", R.cols, R.rows, SDL_PixelFormat::SDL_PIXELFORMAT_RGB24, true);
  std::memcpy(image_result->GetImage(0)->pixels, R3C.data, image_result->GetHeight() * image_result->GetWidth() * 3);

  auto end_time_process = std::chrono::high_resolution_clock::now();

  int64_t tick_count = std::chrono::duration_cast<std::chrono::microseconds>(end_time_process - start_time_process).count();
  timeToComplete = static_cast<float>(tick_count) / 1000.0f;
  timeFilterToComplete = (lpf_smooth_factor * timeToComplete) + (1.0f - lpf_smooth_factor) * timeFilterToComplete;

  return {image_result};
}

void PixelCornersBlock::ExtraEnableProcess()
{
  produceGradXThread = cthread("prod_gx", "produce gradient x", &PixelCornersBlock::GradXTask, this);
  produceGradYThread = cthread("prod_gy", "produce gradient y", &PixelCornersBlock::GradYTask, this);
  produceGradXYThread = cthread("prod_gxy", "produce gradient xy", &PixelCornersBlock::GradXYTask, this);
  produceGradXXThread = cthread("prod_gxx", "produce gradient xx", &PixelCornersBlock::GradXXTask, this);
  produceGradYYThread = cthread("prod_gyy", "produce gradient yy", &PixelCornersBlock::GradYYTask, this);
}

void PixelCornersBlock::ExtraDisableProcess()
{
  if (produceGradXThread.joinable())
  {
    produceGradXThread.join();
  }

  if (produceGradYThread.joinable())
  {
    produceGradYThread.join();
  }

  if (produceGradXYThread.joinable())
  {
    produceGradXYThread.join();
  }

  if (produceGradXXThread.joinable())
  {
    produceGradXXThread.join();
  }

  if (produceGradYYThread.joinable())
  {
    produceGradYYThread.join();
  }
}

void PixelCornersBlock::GradXTask()
{
  while (IsEnabled())
  {
    {
      std::unique_lock lock(mtxReceivedInput);
      receivedInputCondition.wait_for(lock, std::chrono::milliseconds(1000u));
    }

    if (!sourcesGXQueue.empty())
    {
      auto [gray_wrapper, grad_x_wrapper] = sourcesGXQueue.front();
      sourcesGXQueue.pop();

      auto& gray = gray_wrapper.get();
      auto& grad_x = grad_x_wrapper.get();

      cv::Scharr(gray, grad_x, CV_32F, 1, 0);
    }

    gradSync.arrive_and_wait();
  }

  gradSync.arrive_and_drop();
}

void PixelCornersBlock::GradYTask()
{
  while (IsEnabled())
  {
    {
      std::unique_lock lock(mtxReceivedInput);
      receivedInputCondition.wait_for(lock, std::chrono::milliseconds(1000u));
    }

    if (!sourcesGYQueue.empty())
    {
      auto [gray_wrapper, grad_y_wrapper] = sourcesGYQueue.front();
      sourcesGYQueue.pop();

      auto& gray = gray_wrapper.get();
      auto& grad_y = grad_y_wrapper.get();

      cv::Scharr(gray, grad_y, CV_32F, 0, 1);
    }

    gradSync.arrive_and_wait();
  }

  gradSync.arrive_and_drop();
}

void PixelCornersBlock::GradXYTask()
{
  while (IsEnabled())
  {
    {
      std::unique_lock lock(mtxReceivedGrad);
      receivedGradCondition.wait_for(lock, std::chrono::milliseconds(1000u));
    }

    if (!sourcesGXYQueue.empty())
    {
      auto [grad_x_wrapper, grad_y_wrapper, grad_xy_wrapper] = sourcesGXYQueue.front();
      sourcesGXYQueue.pop();

      auto& grad_x = grad_x_wrapper.get();
      auto& grad_y = grad_y_wrapper.get();
      auto& grad_xy = grad_xy_wrapper.get();

      constexpr int32_t blockSize = 3;
      grad_xy = grad_x.mul(grad_y);
      cv::GaussianBlur(grad_xy, grad_xy, cv::Size(blockSize, blockSize), sigmaFactor);
    }

    gradBlurSync.arrive_and_wait();
  }

  gradBlurSync.arrive_and_drop();
}

void PixelCornersBlock::GradXXTask()
{
  while (IsEnabled())
  {
    {
      std::unique_lock lock(mtxReceivedGrad);
      receivedGradCondition.wait_for(lock, std::chrono::milliseconds(1000u));
    }

    if (!sourcesGXXQueue.empty())
    {
      auto [grad_x0_wrapper, grad_x1_wrapper, grad_xx_wrapper] = sourcesGXXQueue.front();
      sourcesGXXQueue.pop();

      auto& grad_x0 = grad_x0_wrapper.get();
      auto& grad_x1 = grad_x1_wrapper.get();
      auto& grad_xx = grad_xx_wrapper.get();

      constexpr int32_t blockSize = 3;
      grad_xx = grad_x0.mul(grad_x1);
      cv::GaussianBlur(grad_xx, grad_xx, cv::Size(blockSize, blockSize), sigmaFactor);
    }

    gradBlurSync.arrive_and_wait();
  }

  gradBlurSync.arrive_and_drop();
}

void PixelCornersBlock::GradYYTask()
{
  while (IsEnabled())
  {
    {
      std::unique_lock lock(mtxReceivedGrad);
      receivedGradCondition.wait_for(lock, std::chrono::milliseconds(1000u));
    }

    if (!sourcesGYYQueue.empty())
    {
      auto [grad_y0_wrapper, grad_y1_wrapper, grad_yy_wrapper] = sourcesGYYQueue.front();
      sourcesGYYQueue.pop();

      auto& grad_y0 = grad_y0_wrapper.get();
      auto& grad_y1 = grad_y1_wrapper.get();
      auto& grad_yy = grad_yy_wrapper.get();

      constexpr int32_t blockSize = 3;
      grad_yy = grad_y0.mul(grad_y1);
      cv::GaussianBlur(grad_yy, grad_yy, cv::Size(blockSize, blockSize), sigmaFactor);
    }

    gradBlurSync.arrive_and_wait();
  }

  gradBlurSync.arrive_and_drop();
}

void PixelCornersBlock::OnPhaseGradCompletion() noexcept
{
  {
    std::unique_lock lock(mtxGrad);
    gradComplete = true;
  }
  produceGradCondition.notify_one();
}

void PixelCornersBlock::OnPhaseGradBlurCompletion() noexcept
{
  {
    std::unique_lock lock(mtxGrad);
    gradBlurComplete = true;
  }
  produceGradBlurCondition.notify_one();
}
