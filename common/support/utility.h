#pragma once

#ifdef _WIN32
#include <windows.h>
#include <consoleapi.h>
#endif

#include <cstring>
#include <string>
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>

namespace utility::opengl {
    inline GLenum GLErrorCheck()
    {
        auto error = glGetError();
        spdlog::error(fmt::format(fmt::fg(fmt::terminal_color::yellow) | fmt::bg(fmt::terminal_color::blue) | fmt::emphasis::bold, "OpenGL Error: 0x{0:x}", error));

        return error;
    }
}

namespace utility{

#if defined(__GNUC__) || defined(__clang__)
    __attribute__((target("avx2")))
#elif defined(_MSC_VER)
#pragma optimize( "gt", on )
#endif

    inline void fast_memcpy(void* dest, const void* src, size_t size)
    {
        std::memcpy(dest, src, size);
    }

#if defined(__GNUC__) || defined(__clang__)
#elif defined(_MSC_VER)
#pragma optimize( "", on )
#endif

    inline int32_t parse_number_from_suffix(std::string_view sv)
    {
        const auto it = std::ranges::find_if(sv, [](const char c) {
            return std::isdigit(static_cast<unsigned char>(c));
        });

        if (it == sv.end()) return -1;

        int32_t value;
        auto result = std::from_chars(&*it, sv.data() + sv.size(), value);
        if (result.ec != std::errc()) return -1;

        return value;
    }

    inline std::string escape_control_chars(const std::string_view& input)
    {
        std::string output;
        for (const char c : input)
        {
            switch (c)
            {
                case '\n': output += "\\n"; break;
                case '\r': output += "\\r"; break;
                case '\t': output += "\\t"; break;
                case '\\': output += "\\\\"; break;
                default: output += c;
            }
        }
        return output;
    }

    inline void EnableConsoleMode()
    {
      #ifdef _WIN32
        HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (handle != INVALID_HANDLE_VALUE) {
          DWORD mode = 0;
          if (GetConsoleMode(handle, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            ::SetConsoleMode(handle, mode);
          }
        }
      #endif
    }
}
