#include "PixelCornersBlock.h"

#include "graphics/images/FImage.h"
#include "examples/camera-streaming/filter-copy-stream-video/filters/EdgeFilters.h"

#include "opencv2/opencv.hpp"
#include "SDL3/SDL.h"

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

  cv::Mat input_rgb(image_source->GetHeight(), image_source->GetWidth(), CV_8UC3, image_source->GetImage(0)->pixels);

  cv::Mat gray;
  cv::cvtColor(input_rgb, gray, cv::COLOR_RGB2GRAY);
  gray.convertTo(gray, CV_32F, 1.0 / 255.0);

  cv::Mat grad_x;
  cv::Mat grad_y;
  cv::Scharr(gray, grad_x, CV_32F, 1, 0);
  cv::Scharr(gray, grad_y, CV_32F, 0, 1);

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

  cv::Mat gray3;
  cv::cvtColor(R, gray3, cv::COLOR_GRAY2RGB);
  gray3.convertTo(gray3, CV_8UC3, 255.0);

  auto image_result = std::make_shared<FImage>("corners", R.cols, R.rows, SDL_PixelFormat::SDL_PIXELFORMAT_RGB24, true);
  std::memcpy(image_result->GetImage(0)->pixels, gray3.data, image_result->GetHeight() * image_result->GetWidth() * 3);

  double minVal, maxVal;
  cv::minMaxLoc(grad_x, &minVal, &maxVal);
  logging::info("grad_x type: {} -- range: [{}, {}]", grad_x.type(), minVal, maxVal);

  cv::minMaxLoc(grad_y, &minVal, &maxVal);
  logging::info("grad_y type: {} -- range: [{}, {}]", grad_y.type(), minVal, maxVal);

  return {image_result};
}
