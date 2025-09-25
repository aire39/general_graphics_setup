#pragma once

#if __cpp_lib_parallel_algorithm || __cpp_lib_execution && __cpp_lib_execution >= 201603 || defined(__GNUC__) && (__GNUC__ > 9 || (__GNUC__ == 9 && __GNUC_MINOR__ >= 0)) || _MSC_VER >= 1926
#include <ranges>
#else
#include <range/v3/all.hpp>
#endif

#if __cpp_lib_parallel_algorithm || __cpp_lib_execution && __cpp_lib_execution >= 201603 || defined(__GNUC__) && (__GNUC__ > 9 || (__GNUC__ == 9 && __GNUC_MINOR__ >= 0)) || _MSC_VER >= 1926
namespace gss {
  namespace views = std::views;
}
#else
namespace gss {
  namespace views = ranges::views;
}
#endif

