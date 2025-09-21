#pragma once

#include "graphics/FilterTypes.h"

namespace filter::functions::cpu::parallel_vectorize {

  constexpr int default_user_value = 0xFF;

  inline filter::types::FilterType varying_bits_process = {[] (const uint8_t * image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> {

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    const uint8_t varying_bit_mask = std::holds_alternative<int>(user_data) ? static_cast<uint8_t>(std::get<int>(user_data)) : 0xFF;

    if (image_data && w && h && p && (bpp == 3))
    {
      r = image_data[(x * bpp) + (y * p) + 0] & varying_bit_mask;
      g = image_data[(x * bpp) + (y * p) + 1] & varying_bit_mask;
      b = image_data[(x * bpp) + (y * p) + 2] & varying_bit_mask;
    }

    return {r, g, b, a};

  }
    , std::execution::par_unseq
    , default_user_value
  };
}
