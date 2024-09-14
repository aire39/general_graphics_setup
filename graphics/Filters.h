#pragma once

#include <cstdint>
#include <functional>
#include "PixelData.h"

namespace filter::types {
    /**
     * General filter type
     *
     * image_data -> image byte data
     * x -> pixel x coordinate
     * y -> pixel y coordinate
     * bpp -> bytes per pixel
     * w -> image width
     * h -> image height
     * p -> number of bytes per row of the image (width * bpp) also called pitch
     */

    using FilterType = std::function<void(uint8_t* image_data, const int& x, const int& y, const int& bpp, const int& w, const int& h, const int& p)>;
}

namespace filter::functions {
    constexpr auto default_filter_process = [](uint8_t * image_data, const int& x, const int& y, const int& bpp, const int& w, const int& h, const int& p) -> void {
        if (image_data && w && h && p && (bpp == 3))
        {
            const uint8_t r = image_data[x + (y * p) + 0];
            const uint8_t g = image_data[x + (y * p) + 1];
            const uint8_t b = image_data[x + (y * p) + 2];

            image_data[x + (y * p) + 0] = r;
            image_data[x + (y * p) + 1] = g;
            image_data[x + (y * p) + 2] = b;
        }
    };

    constexpr auto convert_to_grayscale = [](uint8_t* image_data, const int& x, const int& y, const int& bpp, const int& w, const int& h, const int& p) -> void
    {
        if (h > 0 && w > 0 && bpp == 3)
        {
            const uint32_t index = x + (y * p);
            auto* rgb_color = reinterpret_cast<pixel::data::RGBColor*>(&image_data[index]);

            const uint16_t gray_color = (rgb_color->red + rgb_color->green + rgb_color->blue) / bpp;

            rgb_color->red = gray_color;
            rgb_color->green = gray_color;
            rgb_color->blue = gray_color;
        }
    };
}