#pragma once

#include <cstdint>
#include <functional>
#include <tuple>
#include <variant>
#include <execution>

namespace filter::types {
  /*!
   * General filter type
   *
   * std::tuple
   *
   * first:
   * image_data -> image byte data
   * x -> pixel x coordinate
   * y -> pixel y coordinate
   * bpp -> bytes per pixel
   * w -> image width
   * h -> image height
   * p -> number of bytes per row of the image (width * bpp) also called pitch
   *
   * second:
   * ExecutionPolicies -> type of execution
   *
   * third:
   * FilterUserTypes -> user data types that can be determined {float, int}
   */

  using ExecutionPolicies = std::variant<std::execution::sequenced_policy, std::execution::unsequenced_policy, std::execution::parallel_policy, std::execution::parallel_unsequenced_policy>;
  using FilterUserTypes = std::variant<float, int>;
  using FilterFuncType = std::function<std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>(const uint8_t* image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, FilterUserTypes user_data)>;
  using FilterType = std::tuple<FilterFuncType, ExecutionPolicies, FilterUserTypes>;
}
