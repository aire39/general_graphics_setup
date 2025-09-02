#pragma once

#include <cstring>
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

}
