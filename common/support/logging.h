#pragma once

#include <spdlog/spdlog.h>
#include <source_location>

namespace logging {
  using source_location = std::source_location;
  [[nodiscard]] constexpr auto get_log_source_location(
      const source_location &location) {
    return spdlog::source_loc{location.file_name(),
                              static_cast<std::int32_t>(location.line()),
                              location.function_name()};
  }

  struct format_with_location {
    std::string_view value;
    spdlog::source_loc loc;

    template <typename String>
    format_with_location(const String &s, const source_location &location =
                                              source_location::current())
        : value{s}, loc{get_log_source_location(location)} {}
  };

  template <typename... Args>
  void warn(format_with_location fmt, Args &&...args)
  {
    spdlog::default_logger_raw()->log(fmt.loc, spdlog::level::warn,
                                      fmt::runtime(fmt.value),
                                      std::forward<Args>(args)...);
  }

  template <typename... Args>
  void info(format_with_location fmt, Args &&...args) {
    spdlog::default_logger_raw()->log(fmt.loc, spdlog::level::info,
                                      fmt::runtime(fmt.value),
                                      std::forward<Args>(args)...);
  }

  template <typename... Args>
  void error(format_with_location fmt, Args &&...args) {
    spdlog::default_logger_raw()->log(fmt.loc, spdlog::level::err,
                                      fmt::runtime(fmt.value),
                                      std::forward<Args>(args)...);
  }

  inline void set_logging_output()
  {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%s:%#] [%^%l%$] %v");
  }
}  // namespace logging
