#include "Filters.h"

#include <algorithm>
#include <random>
#include "../images/PixelData.h"

namespace {
    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> DefaultFilterProcess(const uint8_t * image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p)
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;

        if (image_data && w && h && p && (bpp >= 3))
        {
            r = image_data[(x * bpp) + (y * p) + 0];
            g = image_data[(x * bpp) + (y * p) + 1];
            b = image_data[(x * bpp) + (y * p) + 2];
        }

        return {r, g, b, a};
    }

    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> ConvertToGrayScaleProcess(const uint8_t * image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p)
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;

        if (h > 0 && w > 0 && bpp >= 3)
        {
            const auto index = static_cast<uint32_t>((x * bpp) + (y * p));
            const auto* rgb_color = reinterpret_cast<const pixel::data::RGBColor*>(&image_data[index]);

            const auto gray_color = static_cast<uint16_t>((rgb_color->red + rgb_color->green + rgb_color->blue) / bpp);

            r = static_cast<uint8_t>(gray_color);
            g = static_cast<uint8_t>(gray_color);
            b = static_cast<uint8_t>(gray_color);
        }

        return {r, g, b, a};
    }

    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> RandomPixelColorProcess([[maybe_unused]] const uint8_t * image_data, [[maybe_unused]] const int32_t& x, [[maybe_unused]] const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, [[maybe_unused]] const int32_t& p)
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;

        if (h > 0 && w > 0 && bpp >= 3)
        {
            std::random_device m_rand;
            std::mt19937 gen(m_rand());
            std::uniform_int_distribution distribution(0, 255);
            const int random_value_red = distribution(gen);
            const int random_value_green = distribution(gen);
            const int random_value_blue = distribution(gen);

            r = static_cast<uint8_t>(std::ranges::clamp(random_value_red, 0, 255));
            g = static_cast<uint8_t>(std::ranges::clamp(random_value_green, 0, 255));
            b = static_cast<uint8_t>(std::ranges::clamp(random_value_blue, 0, 255));
        }

        return {r, g, b, a};
    }

    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> GuassianBlue3x3KernelProcess(const uint8_t * image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p)
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;

        if (h > 0 && w > 0 && bpp >= 3)
        {
            const int32_t xtl = std::clamp(x - 1, 0, w - 1);
            const int32_t ytl = std::clamp(y - 1, 0, h - 1);
            const int32_t xt = std::clamp(x - 0, 0, w - 1);
            const int32_t yt = std::clamp(y - 1, 0, h - 1);
            const int32_t xtr = std::clamp(x + 1, 0, w - 1);
            const int32_t ytr = std::clamp(y - 1, 0, h - 1);

            const int32_t xl = std::clamp(x - 1, 0, w - 1);
            const int32_t yl = std::clamp(y + 0, 0, h - 1);
            const int32_t xc = std::clamp(x + 0, 0, w - 1);
            const int32_t yc = std::clamp(y + 0, 0, h - 1);
            const int32_t xr = std::clamp(x + 1, 0, w - 1);
            const int32_t yr = std::clamp(y + 0, 0, h - 1);

            const int32_t xbl = std::clamp(x - 1, 0, w - 1);
            const int32_t ybl = std::clamp(y + 1, 0, h - 1);
            const int32_t xb = std::clamp(x - 0, 0, w - 1);
            const int32_t yb = std::clamp(y + 1, 0, h - 1);
            const int32_t xbr = std::clamp(x + 1, 0, w - 1);
            const int32_t ybr = std::clamp(y + 1, 0, h - 1);

            const auto top_left_index = static_cast<uint32_t>((xtl * bpp) + (ytl * p));
            const auto top_index = static_cast<uint32_t>((xt * bpp) + (yt * p));
            const auto top_right_index = static_cast<uint32_t>((xtr * bpp) + (ytr * p));
            const auto left_index = static_cast<uint32_t>((xl * bpp) + (yl * p));
            const auto center_index = static_cast<uint32_t>((xc * bpp) + (yc * p));
            const auto right_index = static_cast<uint32_t>((xr * bpp) + (yr * p));
            const auto bottom_left_index = static_cast<uint32_t>((xbl * bpp) + (ybl * p));
            const auto bottom_index = static_cast<uint32_t>((xb * bpp) + (yb * p));
            const auto bottom_right_index = static_cast<uint32_t>((xbr * bpp) + (ybr * p));

            const auto* rgb_color_tl = reinterpret_cast<const pixel::data::RGBColor*>(&image_data[top_left_index]);
            const auto* rgb_color_t = reinterpret_cast<const pixel::data::RGBColor*>(&image_data[top_index]);
            const auto* rgb_color_tr = reinterpret_cast<const pixel::data::RGBColor*>(&image_data[top_right_index]);
            const auto* rgb_color_l = reinterpret_cast<const pixel::data::RGBColor*>(&image_data[left_index]);
            const auto* rgb_color_c = reinterpret_cast<const pixel::data::RGBColor*>(&image_data[center_index]);
            const auto* rgb_color_r = reinterpret_cast<const pixel::data::RGBColor*>(&image_data[right_index]);
            const auto* rgb_color_bl = reinterpret_cast<const pixel::data::RGBColor*>(&image_data[bottom_left_index]);
            const auto* rgb_color_b = reinterpret_cast<const pixel::data::RGBColor*>(&image_data[bottom_index]);
            const auto* rgb_color_br = reinterpret_cast<const pixel::data::RGBColor*>(&image_data[bottom_right_index]);

            const int32_t blur_color_red = ((rgb_color_tl->red * 1 + rgb_color_t->red * 2 + rgb_color_tr->red * 1) +
                                            (rgb_color_l->red  * 2 + rgb_color_c->red * 4 + rgb_color_r->red  * 2) +
                                            (rgb_color_bl->red * 1 + rgb_color_b->red * 2 + rgb_color_br->red * 1)) / 16;

            const int32_t blur_color_green = ((rgb_color_tl->green * 1 + rgb_color_t->green * 2 + rgb_color_tr->green * 1) +
                                              (rgb_color_l->green  * 2 + rgb_color_c->green * 4 + rgb_color_r->green  * 2) +
                                              (rgb_color_bl->green * 1 + rgb_color_b->green * 2 + rgb_color_br->green * 1)) / 16;

            const int32_t blur_color_blue = ((rgb_color_tl->blue * 1 + rgb_color_t->blue * 2 + rgb_color_tr->blue * 1) +
                                             (rgb_color_l->blue  * 2 + rgb_color_c->blue * 4 + rgb_color_r->blue  * 2) +
                                             (rgb_color_bl->blue * 1 + rgb_color_b->blue * 2 + rgb_color_br->blue * 1)) / 16;

            r = static_cast<uint8_t>(std::clamp(blur_color_red, 0, 255));
            g = static_cast<uint8_t>(std::clamp(blur_color_green, 0, 255));
            b = static_cast<uint8_t>(std::clamp(blur_color_blue, 0, 255));
        }

        return {r, g, b, a};
    }

    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> UYVY2RGBConversion(const uint8_t * image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, [[maybe_unused]] const int32_t& p)
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;

        if (h > 0 && w > 0 && bpp >= 3)
        {
            const int32_t yuv_index = ((x / 2) * 4) + (y * w * 2);

            const int32_t U = image_data[yuv_index  + 0] - 128;
            const int32_t V = image_data[yuv_index  + 2] - 128;
            const int32_t Y = (x % 2 == 0) ? image_data[yuv_index + 1] - 16 : image_data[yuv_index + 3] - 16;

            // conversion factors from https://stackoverflow.com/questions/76713251/converting-uyvy-data-to-rgb
            r = static_cast<uint8_t>(std::clamp(((298 * Y) + (409 * V) + 128) >> 8, 0, 255));
            g = static_cast<uint8_t>(std::clamp(((298 * Y) - (100 * U) - (208 * V) + 128) >> 8, 0, 255));
            b = static_cast<uint8_t>(std::clamp(((298 * Y) + (516 * U) + 128) >> 8, 0, 255));
        }

        return {r, g, b, a};
    }

    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> YUY2RGBConversion(const uint8_t * image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, [[maybe_unused]] const int32_t& p)
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;

        if (h > 0 && w > 0 && bpp >= 3)
        {
            const int32_t yuv_index = ((x / 2) * 4) + (y * w * 2);

            const int32_t U = image_data[yuv_index  + 1] - 128;
            const int32_t V = image_data[yuv_index  + 3] - 128;
            const int32_t Y = (x % 2 == 0) ? image_data[yuv_index + 0] - 16 : image_data[yuv_index + 2] - 16;

            // conversion factors from https://stackoverflow.com/questions/76713251/converting-uyvy-data-to-rgb
            r = static_cast<uint8_t>(std::clamp(((298 * Y) + (409 * V) + 128) >> 8, 0, 255));
            g = static_cast<uint8_t>(std::clamp(((298 * Y) - (100 * U) - (208 * V) + 128) >> 8, 0, 255));
            b = static_cast<uint8_t>(std::clamp(((298 * Y) + (516 * U) + 128) >> 8, 0, 255));
        }

        return {r, g, b, a};
    }

    filter::types::FilterUserTypes default_user_type = 0;
}

namespace filter::functions::cpu::parallel_vectorize {
    filter::types::FilterType default_filter_process = {
        [](const uint8_t * image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> {
            return DefaultFilterProcess(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::par_unseq
        , default_user_type
    };

    filter::types::FilterType convert_to_grayscale = {
        [](const uint8_t* image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>
        {
            return ConvertToGrayScaleProcess(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::par_unseq
        , default_user_type
    };

    filter::types::FilterType random_pixel_colors = {
        []([[maybe_unused]] const uint8_t* image_data, [[maybe_unused]] const int32_t& x, [[maybe_unused]] const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, [[maybe_unused]] const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>
        {
            return RandomPixelColorProcess(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::par_unseq
        , default_user_type
    };

    filter::types::FilterType gaussian_blur_3x3 = {
        [](const uint8_t* image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>
        {
            return GuassianBlue3x3KernelProcess(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::par_unseq
        , default_user_type
    };

    filter::types::FilterType uyvy_to_rgb_conversion = {
        [](const uint8_t* image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>
        {
            return UYVY2RGBConversion(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::par_unseq
        , default_user_type
    };

    filter::types::FilterType yuy2_to_rgb_conversion = {
        [](const uint8_t* image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>
        {
            return YUY2RGBConversion(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::par_unseq
        , default_user_type
    };
}

namespace filter::functions::cpu::sequential {
    filter::types::FilterType default_filter_process = {
        [](const uint8_t * image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> {
            return DefaultFilterProcess(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::seq
        , default_user_type
    };

    filter::types::FilterType convert_to_grayscale = {
        [](const uint8_t* image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>
        {
            return ConvertToGrayScaleProcess(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::seq
        , default_user_type
    };

    filter::types::FilterType random_pixel_colors = {
        []([[maybe_unused]] const uint8_t* image_data, [[maybe_unused]] const int32_t& x, [[maybe_unused]] const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, [[maybe_unused]] const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>
        {
            return RandomPixelColorProcess(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::seq
        , default_user_type
    };

    filter::types::FilterType gaussian_blur_3x3 = {
        [](const uint8_t* image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>
        {
            return GuassianBlue3x3KernelProcess(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::seq
        , default_user_type
    };
}

namespace filter::functions::cpu::vectorize {
    filter::types::FilterType default_filter_process = {
        [](const uint8_t * image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> {
            return DefaultFilterProcess(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::unseq
        , default_user_type
    };

    filter::types::FilterType convert_to_grayscale = {
        [](const uint8_t* image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>
        {
            return ConvertToGrayScaleProcess(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::unseq
        , default_user_type
    };

    filter::types::FilterType random_pixel_colors = {
        []([[maybe_unused]] const uint8_t* image_data, [[maybe_unused]] const int32_t& x, [[maybe_unused]] const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, [[maybe_unused]] const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>
        {
            return RandomPixelColorProcess(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::unseq
        , default_user_type
    };

    filter::types::FilterType gaussian_blur_3x3 = {
        [](const uint8_t* image_data, const int32_t& x, const int32_t& y, const int32_t& bpp, const int32_t& w, const int32_t& h, const int32_t& p, [[maybe_unused]] const filter::types::FilterUserTypes user_data) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>
        {
            return GuassianBlue3x3KernelProcess(image_data, x, y, bpp, w, h, p);
        }
        , filter::types::ExecutionPolicies::unseq
        , default_user_type
    };
}
