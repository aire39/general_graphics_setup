#include "ImageMath.h"

#include <cstdint>
#include <algorithm>
#include <SDL3/SDL.h>
#include "graphics/images/FImage.h"

namespace ggs::image::math {

  std::shared_ptr<FImage> Multiply(const FImage* src_0, const FImage* src_1)
  {
    std::shared_ptr<FImage> new_image = nullptr;

    if (src_0 && src_1)
    {
      auto image_source_surface_0 = src_0->GetImage(src_0->GetFinalImageFilterID());
      auto image_source_surface_1 = src_1->GetImage(src_1->GetFinalImageFilterID());

      int32_t image_width = src_0->GetWidth() < src_1->GetWidth() ? src_0->GetWidth() : src_1->GetWidth();
      int32_t image_height = src_0->GetHeight() < src_1->GetHeight() ? src_0->GetHeight() : src_1->GetHeight();
      int32_t bpp = src_0->GetBytesPerPixel();

      const uint8_t* image_data_0 = static_cast<uint8_t *>(image_source_surface_0->pixels);
      const uint8_t* image_data_1 = static_cast<uint8_t *>(image_source_surface_1->pixels);

      new_image = std::make_shared<FImage>("result", image_width, image_height, SDL_PIXELFORMAT_RGB24, true);
      const auto new_image_surface = new_image->GetImage(src_0->GetFinalImageFilterID());
      const auto new_image_data = static_cast<uint8_t *>(new_image_surface->pixels);

      for (int32_t y = 0; y < image_height; y++)
      {
        for (int32_t x = 0; x < image_width; x++)
        {
          const uint32_t index = static_cast<uint32_t>(x + (y * image_width * bpp));
          constexpr uint32_t red_channel_index = 0;
          constexpr uint32_t green_channel_index = 1;
          constexpr uint32_t blue_channel_index = 2;

          const float red_result = (image_data_0[index + red_channel_index] / 255.0f) * (image_data_1[index + red_channel_index] / 255.0f);
          new_image_data[index + red_channel_index] = static_cast<uint8_t>(std::clamp(red_result * 255.0f, 0.0f, 255.0f));

          const float green_result = (image_data_0[index + red_channel_index] / 255.0f) * (image_data_1[index + green_channel_index] / 255.0f);
          new_image_data[index + red_channel_index] = static_cast<uint8_t>(std::clamp(green_result * 255.0f, 0.0f, 255.0f));

          const float blue_result = (image_data_0[index + red_channel_index] / 255.0f) * (image_data_1[index + blue_channel_index] / 255.0f);
          new_image_data[index + red_channel_index] = static_cast<uint8_t>(std::clamp(blue_result * 255.0f, 0.0f, 255.0f));
        }
      }
    }

    return new_image;
  }

}