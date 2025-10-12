#pragma once

#if defined(__cpp_lib_parallel_algorithm) && __cpp_lib_parallel_algorithm >= 201703L || (defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L) \
&& ((defined(__GNUC__) && __GNUC__ >= 12) || (defined(__clang__) && __clang_major__ > 21) || (defined(_MSC_VER) && _MSC_VER >= 1930))
#include <ranges>
#else
#include <range/v3/all.hpp>
#endif

#if defined(__cpp_lib_parallel_algorithm) && __cpp_lib_parallel_algorithm >= 201703L || (defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L) \
&& ((defined(__GNUC__) && __GNUC__ >= 12) || (defined(__clang__) && __clang_major__ > 21) || (defined(_MSC_VER) && _MSC_VER >= 1930))
namespace gss {
  namespace views = std::views;
}
#else
namespace gss {
  namespace views = ranges::views;
}
#endif

