#include "PixelCornersBlock.h"

#include "graphics/filters/Filters.h"
#include "graphics/images/FImage.h"
#include "examples/camera-streaming/filter-copy-stream-video/filters/EdgeFilters.h"
#include "graphics/operations/ImageMath.h"

PixelCornersBlock::PixelCornersBlock(const std::string &thread_name, const std::string &thread_description)
{
  name = thread_name;
  description = thread_description;
  dataFlow = DataFlow::F_INOUT;
}

PixelCornersBlock::PixelCornersBlock(const std::string &thread_name, const std::string &thread_description, std::shared_ptr<ImageProcessBlock> other)
{
  name = thread_name;
  description = thread_description;
  connection = other;
  dataFlow = DataFlow::F_INOUT;
}

std::vector<std::shared_ptr<FImage>> PixelCornersBlock::Process(std::vector<std::shared_ptr<FImage>> image_sources)
{
  std::shared_ptr<FImage> image_source = nullptr;
  if (!image_sources.empty())
  {
    image_source = image_sources.back();
  }

  auto gradient_x_image = std::make_shared<FImage>("gradient_x image", image_source->GetWidth(), image_source->GetHeight(), SDL_PIXELFORMAT_RGB24, true);
  auto gradient_y_image = std::make_shared<FImage>("gradient_y image", image_source->GetWidth(), image_source->GetHeight(), SDL_PIXELFORMAT_RGB24, true);

  gradient_x_image->ProcessFilterFromImage(image_source.get(), 0, 0, filter::functions::cpu::parallel_vectorize::edge_gradient_x_process);
  gradient_y_image->ProcessFilterFromImage(image_source.get(), 0, 0, filter::functions::cpu::parallel_vectorize::edge_gradient_y_process);

  auto grad_xx = ggs::image::math::Multiply(gradient_x_image.get(), gradient_x_image.get());
  auto grad_yy = ggs::image::math::Multiply(gradient_y_image.get(), gradient_y_image.get());

  auto grad_xy = ggs::image::math::Multiply(gradient_x_image.get(), gradient_y_image.get());

  return {gradient_x_image, gradient_y_image};
}