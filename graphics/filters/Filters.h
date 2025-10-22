#pragma once

#include "FilterTypes.h"

namespace filter::functions::cpu::parallel_vectorize {
    extern types::FilterType default_filter_process;
    extern types::FilterType convert_to_grayscale;
    extern types::FilterType random_pixel_colors;
    extern types::FilterType gaussian_blur_3x3;
    extern types::FilterType uyvy_to_rgb_conversion;
    extern types::FilterType yuy2_to_rgb_conversion;

    //extern types::FilterType yuy2_to_rgb_conversion;
}

namespace filter::functions::cpu::sequential {
    extern types::FilterType default_filter_process;
    extern types::FilterType convert_to_grayscale;
    extern types::FilterType random_pixel_colors;
    extern types::FilterType gaussian_blur_3x3;
}

namespace filter::functions::cpu::vectorize {
    extern types::FilterType default_filter_process;
    extern types::FilterType convert_to_grayscale;
    extern types::FilterType random_pixel_colors;
    extern types::FilterType gaussian_blur_3x3;
}
