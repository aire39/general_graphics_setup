#include "PixelEdgeBlock.h"

#include "graphics/filters/Filters.h"
#include "graphics/images/FImage.h"
#include "examples/camera-streaming/filter-copy-stream-video/filters/EdgeFilters.h"

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

  auto edged_image = std::make_shared<FImage>("edged image", image_source->GetWidth(), image_source->GetHeight(), SDL_PIXELFORMAT_RGB24, true);
  edged_image->ProcessFilterFromImage(image_source.get(), 0, 0, filter::functions::cpu::parallel_vectorize::sobel_edge_process);

  return {edged_image};
}
