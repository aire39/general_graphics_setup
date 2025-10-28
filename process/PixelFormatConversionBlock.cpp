#include "PixelFormatConversionBlock.h"

#include <chrono>
#include "graphics/images/FImage.h"
#include "graphics/filters/Filters.h"

namespace {
  constexpr float lpf_smooth_factor = 0.1f;
}

PixelFormatConversionBlock::PixelFormatConversionBlock(const std::string &thread_name, const std::string &thread_description)
{
  name = thread_name;
  description = thread_description;
  dataFlow = DataFlow::F_INOUT;
}

PixelFormatConversionBlock::PixelFormatConversionBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others)
{
  name = thread_name;
  description = thread_description;
  connections = others;
  dataFlow = DataFlow::F_INOUT;
}

void PixelFormatConversionBlock::SetFormatConversionType(gss::video::camera::types::VideoFormat video_format)
{
  std::lock_guard lock(mtxChangeFormat);
  videoFormat = video_format;
}

gss::video::camera::types::VideoFormat PixelFormatConversionBlock::GetFormatConversionType() const
{
  return videoFormat;
}

std::vector<std::shared_ptr<FImage>> PixelFormatConversionBlock::Process(std::vector<std::shared_ptr<FImage>> image_sources)
{
  std::shared_ptr<FImage> image_source = nullptr;
  if (!image_sources.empty())
  {
    image_source = image_sources.back();
  }

  const auto start_time_process = std::chrono::high_resolution_clock::now();

  auto converted_image = std::make_shared<FImage>("converted image", image_source->GetWidth(), image_source->GetHeight(), SDL_PIXELFORMAT_RGB24, true);

  std::unique_lock lock(mtxChangeFormat);
  if (videoFormat == gss::video::camera::formats::UYVY_FORMAT)
  {
    converted_image->ProcessFilterFromImage(image_source.get(), 0, 0, filter::functions::cpu::parallel_vectorize::uyvy_to_rgb_conversion);
  }
  else if (videoFormat == gss::video::camera::formats::YUY2_FORMAT)
  {
    converted_image->ProcessFilterFromImage(image_source.get(), 0, 0, filter::functions::cpu::parallel_vectorize::yuy2_to_rgb_conversion);
  }
  else // as is
  {
    converted_image->ProcessFilterFromImage(image_source.get(), 0, 0, filter::functions::cpu::parallel_vectorize::default_filter_process);
  }

  lock.unlock();

  // timing information

  const auto end_time_process = std::chrono::high_resolution_clock::now();
  const int64_t tick_count = std::chrono::duration_cast<std::chrono::microseconds>(end_time_process - start_time_process).count();
  timeToComplete = static_cast<float>(tick_count) / 1000.0f;
  timeFilterToComplete = (lpf_smooth_factor * timeToComplete) + (1.0f - lpf_smooth_factor) * timeFilterToComplete;

  return {converted_image};
}
