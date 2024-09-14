#pragma once

#include <string>
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>

namespace utils::openglutils {
    constexpr GLenum GLErrorCheck()
    {
        auto error = glGetError();
        spdlog::error(fmt::format(fmt::fg(fmt::terminal_color::yellow) | fmt::bg(fmt::terminal_color::blue) | fmt::emphasis::bold, "OpenGL Error: 0x{0:x}", error));

        return error;
    }
}
