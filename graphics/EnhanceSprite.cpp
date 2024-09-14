#include "EnhanceSprite.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>
#include "SDL3_image/SDL_image.h"

void EnhanceSprite::LoadTexture(const std::string &image_file)
{
    originalImage = IMG_Load(image_file.c_str());
    baseImage = SDL_ConvertSurface(originalImage, originalImage->format);
    filteredImageDataID.insert({"0", 0});
    Sprite::LoadTexture(baseImage);
}

void EnhanceSprite::LoadTexture(const SDL_Surface *image)
{
    originalImage = SDL_ConvertSurface(const_cast<SDL_Surface*>(image), image->format);
    baseImage = SDL_ConvertSurface(const_cast<SDL_Surface*>(image), image->format);
    filteredImageDataID.insert({"0", 0});
    Sprite::LoadTexture(image);
}

void EnhanceSprite::ProcessFilter(const filter::types::FilterType& filter, const bool save_filter)
{
    if (filteredImageData.empty() && !save_filter)
    {
        ProcessFilter(filter, 0, save_filter);
    }
    else
    {
        if (save_filter)
        {
            filterProcessCalls.push_back(filter);
        }

        ProcessFilter(filter, static_cast<int32_t>(filteredImageData.size()-1), save_filter);
    }
}

void EnhanceSprite::ProcessFilter(const filter::types::FilterType& filter, const int32_t image_id, const bool save_filter)
{
    std::lock_guard lock(mutex);

    if (save_filter)
    {
        SDL_Surface* copy = SDL_ConvertSurface(baseImage, baseImage->format);
        filteredImageData.push_back(copy);

        const uint32_t n_filters = filteredImageData.size();
        filteredImageDataID.insert({std::to_string(n_filters), n_filters});
    }
    else
    {
        if (filterProcessCalls.empty())
        {
            filterProcessCalls.emplace_back(filter::functions::default_filter_process);
        }
        else
        {
            filterProcessCalls[image_id] = filter;
        }
    }

    const SDL_Surface* image = image_id == 0 ? baseImage : filteredImageData[image_id];

    if (image)
    {
        const int32_t bytes_per_pixel = (baseImage->pitch / baseImage->w);
        for (int i=0; i<(baseImage->h); i++)
        {
            for (int j=0; j<baseImage->pitch; j += bytes_per_pixel)
            {
                filter(static_cast<uint8_t*>(image->pixels), j, i, bytes_per_pixel, image->w, image->h, image->pitch);
            }
        }

        texture.Update(static_cast<uint8_t*>(image->pixels), image->w, image->h);
    }
    else
    {
        spdlog::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "No data to appy filter!"));
    }
}

void EnhanceSprite::ProcessFilter(const filter::types::FilterType& filter, const std::string& filter_name, bool save_filter)
{
    try
    {
        const int32_t image_id = filteredImageDataID.at(filter_name);
        ProcessFilter(filter, image_id, save_filter);
    }
    catch (const std::exception& e)
    {
        spdlog::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Unable to find filtered image, exception msg: {}", e.what()));
    }
}

void EnhanceSprite::ChangeFilterName(const std::string &filter_name, const std::string &new_filter_name)
{
    std::lock_guard lock(mutex);

    int32_t filter_id = filteredImageDataID[filter_name];
    filteredImageDataID.erase(filter_name);
    filteredImageDataID.insert({new_filter_name, filter_id});
}

void EnhanceSprite::ChangeFilterName(const int32_t filter_id, const std::string &new_filter_name)
{
    std::lock_guard lock(mutex);

    for (const auto& [name, id] : filteredImageDataID)
    {
        if (filter_id == id)
        {
            std::string filter_name = name;
            filteredImageDataID.erase(filter_name);
            filteredImageDataID.insert({new_filter_name, filter_id});
        }
    }
}

void EnhanceSprite::ViewTexture(const uint32_t filter_id) const
{
    if (!filteredImageData.empty() && filter_id < filteredImageData.size())
    {
        const SDL_Surface* image = filteredImageData[filter_id];
        texture.Update(static_cast<uint8_t*>(image->pixels), image->w, image->h);
    }
    else
    {
        spdlog::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Filtered texture doesn't exist!"));
    }
}

void EnhanceSprite::ViewTexture(const std::string& filter_name) const
{
    try
    {
        const int32_t filter_id = filteredImageDataID.at(filter_name);
        const SDL_Surface* image = filteredImageData[filter_id];
        texture.Update(static_cast<uint8_t*>(image->pixels), image->w, image->h);
    }
    catch (const std::exception& e)
    {
        spdlog::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Filtered texture doesn't exist, exception msg: {}", e.what()));
    }
}

void EnhanceSprite::ResetFilters()
{
    std::lock_guard lock(mutex);

    baseImage = SDL_ConvertSurface(originalImage, originalImage->format);

    filteredImageDataID.clear();
    filteredImageDataID.insert({"0", 0});

    filteredImageData.clear();
    filterProcessCalls.clear();
}
