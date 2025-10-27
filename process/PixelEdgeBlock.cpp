#include "PixelEdgeBlock.h"

#include "graphics/filters/Filters.h"
#include "graphics/images/FImage.h"
#include "examples/camera-streaming/filter-copy-stream-video/filters/EdgeFilters.h"

namespace {
  constexpr float lpf_smooth_factor = 0.1f;
}

PixelEdgeBlock::PixelEdgeBlock(const std::string &thread_name, const std::string &thread_description)
{
  name = thread_name;
  description = thread_description;
  dataFlow = DataFlow::F_INOUT;
}

PixelEdgeBlock::PixelEdgeBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others)
{
  name = thread_name;
  description = thread_description;
  connections = others;
  dataFlow = DataFlow::F_INOUT;
}

std::vector<std::shared_ptr<FImage>> PixelEdgeBlock::Process(std::vector<std::shared_ptr<FImage>> image_sources)
{
  std::shared_ptr<FImage> image_source = nullptr;
  if (!image_sources.empty())
  {
    image_source = image_sources.back();
  }

  const auto start_time_process = std::chrono::high_resolution_clock::now();

  auto edged_image = std::make_shared<FImage>("edged image", image_source->GetWidth(), image_source->GetHeight(), SDL_PIXELFORMAT_RGB24, true);
  edged_image->ProcessFilterFromImage(image_source.get(), 0, 0, filter::functions::cpu::parallel_vectorize::sobel_edge_process);

  const auto end_time_process = std::chrono::high_resolution_clock::now();
  const int64_t tick_count = std::chrono::duration_cast<std::chrono::microseconds>(end_time_process - start_time_process).count();
  timeToComplete = static_cast<float>(tick_count) / 1000.0f;
  timeFilterToComplete = (lpf_smooth_factor * timeToComplete) + (1.0f - lpf_smooth_factor) * timeFilterToComplete;

  return {edged_image};
}
