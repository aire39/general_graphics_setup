#include "PixelFormatConversionBlock.h"

#include "graphics/images/FImage.h"
#include "graphics/filters/Filters.h"
#include "support/logging.h"

PixelFormatConversionBlock::PixelFormatConversionBlock(const std::string &thread_name, const std::string &thread_description)
{
  name = thread_name;
  description = thread_description;
  dataFlow = DataFlow::F_INOUT;
}

PixelFormatConversionBlock::PixelFormatConversionBlock(const std::string &thread_name, const std::string &thread_description, std::shared_ptr<ImageProcessBlock> other)
{
  name = thread_name;
  description = thread_description;
  connection = other;
  dataFlow = DataFlow::F_INOUT;
}

std::vector<std::shared_ptr<FImage>> PixelFormatConversionBlock::Process(std::vector<std::shared_ptr<FImage>> image_sources)
{
  std::shared_ptr<FImage> image_source = nullptr;
  if (!image_sources.empty())
  {
    image_source = image_sources.back();
  }

  auto converted_image = std::make_shared<FImage>("converted image", image_source->GetWidth(), image_source->GetHeight(), SDL_PIXELFORMAT_RGB24, true);
  converted_image->ProcessFilterFromImage(image_source.get(), 0, 0, filter::functions::cpu::parallel_vectorize::uyvy_to_rgb_conversion);

  return {converted_image};
}
