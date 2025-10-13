#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace pixel::data {
    struct RGBColor {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    struct RGBAColor : RGBColor
    {
        uint8_t alpha;
    };
}

namespace pixel::conversion {
    inline glm::vec3 IntegerToFloat(const uint8_t r, const uint8_t g, const uint8_t b)
    {
        float red = static_cast<float>(r) / 255.0f;
        float green = static_cast<float>(g) / 255.0f;
        float blue = static_cast<float>(b) / 255.0f;

        return {red, green, blue};
    }

    inline glm::vec4 IntegerToFloat(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
    {
        float red = static_cast<float>(r) / 255.0f;
        float green = static_cast<float>(g) / 255.0f;
        float blue = static_cast<float>(b) / 255.0f;
        float alpha = static_cast<float>(a) / 255.0f;

        return {red, green, blue, alpha};
    }
}