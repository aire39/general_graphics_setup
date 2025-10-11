#pragma once

#include "graphics/FilterTypes.h"
#include "common/support/utility.h"

namespace filter::functions::cpu::parallel_vectorize {

  inline int32_t convolution(const uint8_t* source, const int8_t* kernel, const int32_t x, const int32_t y, const int32_t channel, const int32_t kernel_size, const int32_t width, const int32_t height, const int32_t bpp)
  {
    int32_t result = 0;

    if (source && kernel)
    {
      const int32_t half_k = kernel_size / 2;
      const int32_t source_size = width * height * bpp;

      for (int32_t i = -half_k; i <= half_k; i++)
      {
        for (int32_t j = -half_k; j <= half_k; j++)
        {
          const int32_t pixel_index = ((x + j) * bpp + channel) + ((y + i) * width * bpp);
          const int32_t kernel_index = (j + half_k) + ((i + half_k) * kernel_size);

          if (pixel_index >= 0 && pixel_index < source_size)
          {
            result += source[pixel_index] * kernel[kernel_index];
          }
        }
      }
    }

    return result / (kernel_size * kernel_size);
  }

  inline filter::types::FilterType edge_process = {[] (const uint8_t * image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> {

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    constexpr int8_t kernel_matrix_x[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    constexpr int8_t kernel_matrix_y[] = {1, 2, 1, 0, 0, 0, -1, -2, -1};

    if (image_data && w && h && p && (bpp == 3))
    {
      constexpr int default_kernel_size = 3;
      r = utility::clamp_to_byte(convolution(image_data, kernel_matrix_x, x, y, 0, default_kernel_size, w, h, bpp) + convolution(image_data, kernel_matrix_y, x, y, 0, default_kernel_size, w, h, bpp));
      g = utility::clamp_to_byte(convolution(image_data, kernel_matrix_x, x, y, 1, default_kernel_size, w, h, bpp) + convolution(image_data, kernel_matrix_y, x, y, 1, default_kernel_size, w, h, bpp));
      b = utility::clamp_to_byte(convolution(image_data, kernel_matrix_x, x, y, 2, default_kernel_size, w, h, bpp) + convolution(image_data, kernel_matrix_y, x, y, 2, default_kernel_size, w, h, bpp));
    }

    return {r, g, b, a};

  }
    , filter::types::ExecutionPolicies::par_unseq
    , 0
  };
}
