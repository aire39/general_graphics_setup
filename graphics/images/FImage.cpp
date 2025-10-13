#include "FImage.h"

#include <algorithm>
#include <execution>
#include <cctype>
#include <ranges>
#include <span>
#include <future>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>
#include <SDL3_image/SDL_image.h>

#include "support/utility.h"
#include "support/cprocess.h"
#include "support/ctypes.h"
#include "../filters/Filters.h"

FImage::FImage()
  : Sprite("FImage")
{
}

FImage::FImage(const std::string& new_name)
  : Sprite(new_name)
{
}

FImage::FImage(const std::string &new_name, const int32_t width, const int32_t height, const SDL_PixelFormat format, const bool base_same_as_orig)
  : Sprite(new_name)
{
    originalImage = std::shared_ptr<SDL_Surface>(SDL_CreateSurface(width, height, format), &FImage::privDeleteSurface);

    if (base_same_as_orig)
    {
        filteredImageData.emplace_back(originalImage);
    }
    else
    {
        filteredImageData.emplace_back(std::shared_ptr<SDL_Surface>(SDL_ConvertSurface(originalImage.get(), originalImage->format), &FImage::privDeleteSurface));
    }

    filteredImageDataID.insert({"base", 0});

    filterProcessCalls.emplace_back(filter::functions::cpu::parallel_vectorize::default_filter_process);
    Sprite::LoadTexture(filteredImageData[0].get());
}

FImage::~FImage()
{
    ResetSprite();
}

FImage::FImage(const FImage &other) noexcept
  : Sprite("FImage")
{
    currentViewLayer = other.currentViewLayer;
    originalImage = std::shared_ptr<SDL_Surface>(SDL_ConvertSurface(other.originalImage.get(), other.originalImage->format), &FImage::privDeleteSurface);
    tmpBuffer = std::shared_ptr<SDL_Surface>(SDL_ConvertSurface(other.tmpBuffer.get(), other.tmpBuffer->format), &FImage::privDeleteSurface);
    hasViewChangedToInProcessFilter = other.hasViewChangedToInProcessFilter;

    for (auto other_image : other.filteredImageData)
    {
        auto new_image = std::shared_ptr<SDL_Surface>(SDL_ConvertSurface(other_image.get(), other_image->format), &FImage::privDeleteSurface);
        filteredImageData.emplace_back(new_image);
    }

    filterProcessCalls = other.filterProcessCalls;
    filteredImageDataID = other.filteredImageDataID;
    shouldReapplyFilters = other.shouldReapplyFilters;
}

FImage::FImage(FImage &&other) noexcept
  : Sprite("FImage")
{
    currentViewLayer = other.currentViewLayer;
    originalImage = std::shared_ptr<SDL_Surface>(SDL_ConvertSurface(other.originalImage.get(), other.originalImage->format), &FImage::privDeleteSurface);
    tmpBuffer = std::shared_ptr<SDL_Surface>(SDL_ConvertSurface(other.tmpBuffer.get(), other.tmpBuffer->format), &FImage::privDeleteSurface);
    hasViewChangedToInProcessFilter = other.hasViewChangedToInProcessFilter;

    for (auto other_image : other.filteredImageData)
    {
        auto new_image = std::shared_ptr<SDL_Surface>(SDL_ConvertSurface(other_image.get(), other_image->format), &FImage::privDeleteSurface);
        filteredImageData.emplace_back(new_image);
    }

    filterProcessCalls = other.filterProcessCalls;
    filteredImageDataID = other.filteredImageDataID;
    shouldReapplyFilters = other.shouldReapplyFilters;
}

void FImage::LoadTexture(const std::string &image_file)
{
    if (!originalImage)
    {
        originalImage.reset();
        ResetSprite();
    }

    originalImage = std::shared_ptr<SDL_Surface>(IMG_Load(image_file.c_str()), &FImage::privDeleteSurface);
    filteredImageData.emplace_back(SDL_ConvertSurface(originalImage.get(), originalImage->format), &FImage::privDeleteSurface);
    filteredImageDataID.insert({"base", 0});

    filterProcessCalls.emplace_back(filter::functions::cpu::parallel_vectorize::default_filter_process);
    Sprite::LoadTexture(filteredImageData[0].get());
}

void FImage::LoadTexture(const SDL_Surface *image)
{
    if (!originalImage)
    {
        originalImage.reset();
        ResetSprite();
    }

    originalImage = std::shared_ptr<SDL_Surface>(SDL_ConvertSurface(const_cast<SDL_Surface*>(image), image->format), &FImage::privDeleteSurface);
    filteredImageData.emplace_back(SDL_ConvertSurface(originalImage.get(), originalImage->format), &FImage::privDeleteSurface);
    filteredImageDataID.insert({"base", 0});

    filterProcessCalls.emplace_back(filter::functions::cpu::parallel_vectorize::default_filter_process);
    Sprite::LoadTexture(image);
}

bool FImage::ProcessFilter(const filter::types::FilterType& filter, const bool save_filter, const int32_t repeat)
{
    const bool valid_image = privProcessFilter(filter, save_filter, repeat) > -1;
    return valid_image;
}

bool FImage::ProcessFilter(const int32_t image_id, const filter::types::FilterType& filter, const bool save_filter, const int32_t repeat)
{
    const bool valid_image = privProcessFilter(image_id, filter, save_filter, repeat) > -1;
    return valid_image;
}

bool FImage::ProcessFilter(const std::string& filter_name, const filter::types::FilterType& filter, const bool save_filter, const int32_t repeat)
{
    const bool valid_image = privProcessFilter(filter_name, filter, save_filter, repeat) > -1;
    return valid_image;
}

bool FImage::ProcessFilter(const std::string& filter_name, const std::string &filter_name_to, const filter::types::FilterType &filter, const int32_t repeat)
{
    const bool valid_image = privProcessFilter(filter_name, filter_name_to, filter, repeat) > -1;
    return valid_image;
}

void FImage::SetPixel(const int32_t image_id, const int32_t x, const int32_t y, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) const
{
    if (!filteredImageData.empty() && image_id >= 0 && image_id < static_cast<glm::int32>(filteredImageData.size()) && filteredImageData[image_id] && (x >= 0) && (x <= filteredImageData[image_id]->w) && (y >= 0) && (y <= filteredImageData[image_id]->h - 1))
    {
        const uint8_t bytes_per_pixel = SDL_GetPixelFormatDetails(filteredImageData[image_id]->format)->bytes_per_pixel;
        const auto filter_image = static_cast<uint8_t*>(filteredImageData[currentViewLayer]->pixels);
        const auto pixel = &filter_image[(x * bytes_per_pixel) + (y * filteredImageData[currentViewLayer]->pitch)];

        pixel[0] = r;
        pixel[1] = g;
        pixel[2] = b;

        if (bytes_per_pixel == 4)
        {
            pixel[3] = a;
        }
    }
    else
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Pixel position to set is out of bounds!"));
    }
}

void FImage::SetPixel(const int32_t x, const int32_t y, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) const
{
    SetPixel(currentViewLayer, x, y, r, g, b, a);
}

void FImage::SetPixel(const int32_t image_id, const glm::ivec2 pos, const glm::u8vec4 color) const
{
    SetPixel(image_id, pos.x, pos.y, color.r, color.g, color.b, color.a);
}

void FImage::SetPixel(const glm::ivec2 pos, const glm::u8vec4 color) const
{
    SetPixel(currentViewLayer, pos.x, pos.y, color.r, color.g, color.b, color.a);
}

glm::u8vec4 FImage::GetPixel(const int32_t image_id, const int32_t x, const int32_t y) const
{
    glm::u8vec4 pixel;

    if (!filteredImageData.empty() && image_id >= 0 && image_id < static_cast<glm::int32>(filteredImageData.size()) && filteredImageData[image_id] && (x >= 0) && (x <= filteredImageData[image_id]->w) && (y >= 0) && (y <= filteredImageData[image_id]->h - 1))
    {
        const uint8_t bytes_per_pixel = SDL_GetPixelFormatDetails(filteredImageData[image_id]->format)->bytes_per_pixel;
        const auto filter_image = static_cast<uint8_t*>(filteredImageData[image_id]->pixels);
        const auto pixel_value = &filter_image[(x * bytes_per_pixel) + (y * filteredImageData[image_id]->pitch)];

        pixel[0] = pixel_value[0];
        pixel[1] = pixel_value[1];
        pixel[2] = pixel_value[2];

        if (bytes_per_pixel == 4)
        {
            pixel[3] = pixel_value[3];
        }
        else
        {
            pixel[3] = 255;
        }
    }
    else
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Pixel position to get is out of bounds!"));
    }

    return pixel;
}

glm::u8vec4 FImage::GetPixel(const int32_t x, const int32_t y) const
{
    return GetPixel(currentViewLayer, x, y);
}

uint8_t FImage::GetRedPixel(const int32_t x, const int32_t y) const
{
    return  GetRedPixel(currentViewLayer, x, y);
}

uint8_t FImage::GetRedPixel(const int32_t image_id, const int32_t x, const int32_t y) const
{
    const glm::u8vec4 pixel = GetPixel(image_id, x, y);
    return pixel.r;
}

uint8_t FImage::GetGreenPixel(const int32_t x, const int32_t y) const
{
    return GetGreenPixel(currentViewLayer, x, y);
}

uint8_t FImage::GetGreenPixel(const int32_t image_id, const int32_t x, const int32_t y) const
{
    const glm::u8vec4 pixel = GetPixel(image_id, x, y);
    return pixel.g;
}

uint8_t FImage::GetBluePixel(const int32_t x, const int32_t y) const
{
    return GetBluePixel(currentViewLayer, x, y);
}

uint8_t FImage::GetBluePixel(const int32_t image_id, const int32_t x, const int32_t y) const
{
    const glm::u8vec4 pixel = GetPixel(image_id, x, y);
    return pixel.b;
}

uint8_t FImage::GetAlphaPixel(const int32_t x, const int32_t y) const
{
    const glm::u8vec4 pixel = GetPixel(x, y);
    return pixel.a;
}

SDL_Surface * FImage::GetImage(const int32_t image_id) const
{
    SDL_Surface* image = nullptr;

    if (!filteredImageData.empty() && image_id >= 0 && image_id < static_cast<glm::int32>(filteredImageData.size()) && filteredImageData[image_id])
    {
        image = filteredImageData[image_id].get();
    }
    else
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "image data does not exists: out of bounds!"));
    }

    return image;
}

void FImage::ChangeFilterName(const std::string &filter_name, const std::string &new_filter_name)
{
    std::lock_guard lock(mutex);

    try
    {
        if (!filteredImageDataID.contains(new_filter_name))
        {
            int32_t filter_id = filteredImageDataID.at(filter_name);
            filteredImageDataID.erase(filter_name);
            filteredImageDataID.insert({new_filter_name, filter_id});
        }
        else
        {
            logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Filter name \"{}\" already exists. Unable to change name", new_filter_name));
        }
    }
    catch (const std::out_of_range &e)
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Filter doesn't exist. Unable to change name -> exception msg: {}", e.what()));
    }
}

void FImage::ChangeFilterName(const int32_t filter_id, const std::string &new_filter_name)
{
    std::lock_guard lock(mutex);

    bool found_filter = false;
    bool found_unique_name = false;

    for (const auto&[filter_name, image_id] : filteredImageDataID)
    {
        if (filter_id == image_id)
        {
            found_filter = true;

            if (!filteredImageDataID.contains(new_filter_name))
            {
                filteredImageDataID.erase(filter_name);
                filteredImageDataID.insert({new_filter_name, filter_id});
                found_unique_name = true;
            }
            break;
        }
    }

    if (!found_filter)
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Filter doesn't exist. Unable to change name"));
    }

    if (!found_unique_name)
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Filter name \"{}\" already exists. Unable to change name", new_filter_name));
    }
}

void FImage::UpdateImageFilter(const int32_t image_id)
{
    if (image_id >= 0 && image_id < static_cast<glm::int32>(filteredImageData.size()))
    {
        constexpr bool save_filter = false;
        ProcessFilter(image_id, filterProcessCalls[image_id], save_filter);
    }
}

void FImage::ViewTexture()
{
    std::lock_guard viewtex_lock(updateTextureMutex);
    const auto image = filteredImageData[currentViewLayer];
    texture.Update(static_cast<uint8_t*>(image->pixels), image->w, image->h);
}

void FImage::ViewTexture(const int32_t filter_id)
{
    std::lock_guard viewtex_lock(updateTextureMutex);
    if (!filteredImageData.empty() && filter_id >= 0 && filter_id < static_cast<int32_t>(filteredImageData.size()))
    {
        const SDL_Surface* image = filteredImageData[filter_id].get();
        texture.Update(static_cast<uint8_t*>(image->pixels), image->w, image->h);

        currentViewLayer = filter_id;
    }
    else
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Filtered texture doesn't exist!"));
    }
}

void FImage::ViewTexture(const std::string& filter_name)
{
    std::lock_guard viewtex_lock(updateTextureMutex);
    try
    {
        std::string filter_name_lower = filter_name;
        std::ranges::transform(filter_name_lower, filter_name_lower.begin(), [](const unsigned char ch) { return std::tolower(ch); });
        const int32_t filter_id = filteredImageDataID.at(filter_name_lower);
        const SDL_Surface* image = filteredImageData[filter_id].get();
        texture.Update(static_cast<uint8_t*>(image->pixels), image->w, image->h);

        currentViewLayer = filter_id;
    }
    catch (const std::exception& e)
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Filtered texture doesn't exist, exception msg: {}", e.what()));
    }
}

int32_t FImage::GetBaseImageFilterID() const
{
    const int32_t image_id = filteredImageDataID.at("base");
    return image_id;
}

int32_t FImage::GetCurrentImageFilterID() const
{
    return currentViewLayer;
}

int32_t FImage::GetFinalImageFilterID() const
{
    return static_cast<int32_t>(filteredImageData.size() - 1);
}

std::vector<std::pair<std::string, int32_t>> FImage::GetProcessedImageNames() const
{
    std::vector<std::pair<std::string, int32_t>> names;

    for (const auto & [image_name, image_index]: filteredImageDataID)
    {
        names.emplace_back(image_name, image_index);
    }

    std::ranges::sort(names, [] (const std::pair<std::string, int32_t>& first, const std::pair<std::string, int32_t>& second) {
        return first.second < second.second;
    });

    return names;
}

std::string FImage::GetProcessedImageNameByID(const int32_t image_id) const
{
    std::string found_name;

    std::vector<std::pair<std::string, int32_t>> processed_names = GetProcessedImageNames();
    for (const auto & [image_name, image_index]: processed_names)
    {
        if (image_id == image_index)
        {
            found_name = image_name;
            break;
        }
    }

    return found_name;
}

int32_t FImage::GetWidth() const
{
    return originalImage->w;
}

int32_t FImage::GetHeight() const
{
    return originalImage->h;
}

int32_t FImage::GetBytesPerPixel() const
{
    return SDL_GetPixelFormatDetails(originalImage->format)->bytes_per_pixel;
}

uint32_t FImage::GetNumImages() const
{
    return static_cast<uint32_t>(filteredImageData.size());
}

void FImage::SetShouldReapplyFilters(const bool enable)
{
    shouldReapplyFilters = enable;
}

void FImage::Remove()
{
    Remove(currentViewLayer);
}

void FImage::Remove(const int32_t image_id)
{
    if (image_id > 0 && image_id < static_cast<int32_t>(filteredImageData.size()))
    {
        {
            inProgressOfUpdatingFilters = false;
            std::unique_lock reapplyFilterLock(reapplyFilterMutex);
            cvWaitForThreadToComplete.wait(reapplyFilterLock, [this]{ return isProgressThreadComplete; });
        }

        filteredImageData[image_id].reset();
        filteredImageData.erase(filteredImageData.begin() + image_id);
        filterProcessCalls.erase(filterProcessCalls.begin() + image_id);

        const std::string image_name = GetProcessedImageNameByID(image_id);
        filteredImageDataID.erase(image_name);
        repeatFilteredImageMap.erase(image_id);

        if (currentViewLayer >= image_id)
        {
            currentViewLayer--;
        }

        filteredImageDataID = RemapIDs(filteredImageDataID, image_id, RenameOrder::REMOVING);
        repeatFilteredImageMap = RemapRepeatIDs(repeatFilteredImageMap, image_id, RenameOrder::REMOVING);
    }
}

void FImage::Remove(const std::string& image_name)
{
    try
    {
        const int32_t image_id = filteredImageDataID.at(image_name);
        Remove(image_id);
    }
    catch (const std::exception& e)
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Unable to find filtered image '{}', exception msg: {}", image_name, e.what()));
    }
}

std::shared_ptr<SDL_Surface> FImage::Extract()
{
    return Extract(currentViewLayer);
}

std::shared_ptr<SDL_Surface> FImage::Extract(const int32_t image_id)
{
    std::shared_ptr<SDL_Surface> extracted_image = nullptr;

    if (image_id > 0 && image_id < static_cast<int32_t>(filteredImageData.size()))
    {
        extracted_image = filteredImageData[image_id];
        Remove(image_id);
    }

    return extracted_image;
}

std::shared_ptr<SDL_Surface> FImage::Extract(const std::string& image_name)
{
    std::shared_ptr<SDL_Surface> extracted_image = nullptr;

    try
    {
        const int32_t image_id = filteredImageDataID.at(image_name);
        extracted_image = Extract(image_id);
    }
    catch (const std::exception& e)
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Unable to find filtered image '{}', exception msg: {}", image_name, e.what()));
    }

    return extracted_image;
}

void FImage::ResetFilters()
{
    std::lock_guard lock(mutex);

    for (auto image : filteredImageData)
    {
        image.reset();
    }

    currentViewLayer = 0;

    filteredImageData.clear();
    filteredImageData.emplace_back(SDL_ConvertSurface(originalImage.get(), originalImage->format));

    filteredImageDataID.clear();
    filteredImageDataID.insert({"base", 0});

    filterProcessCalls.clear();
}

void FImage::Draw()
{
    if (hasViewChangedToInProcessFilter)
    {
        hasViewChangedToInProcessFilter = false;
        ViewTexture();
    }

    Sprite::Draw();
}

void FImage::ResetSprite()
{
    for (auto image : filteredImageData)
    {
        image.reset();
    }

    currentViewLayer = 0;
    filteredImageDataID.clear();
    filteredImageData.clear();
    filterProcessCalls.clear();
}

std::unordered_map<std::string, int32_t> FImage::RemapIDs(const std::unordered_map<std::string, int32_t> &source, const int32_t image_id, const RenameOrder rename_order)
{
    decltype(filteredImageDataID) tmp;
    std::ranges::transform(source, std::inserter(tmp, tmp.begin()) , [&](const std::pair<std::string, int32_t> & v) {

        std::string update_key = v.first;
        int32_t update_id = v.second;

        if (update_key.empty())
        {
            update_id = image_id + 1;
        }
        else if ((v.second > (image_id + 1) || ((image_id + 1) < static_cast<int32_t>(filteredImageDataID.size()) && v.second > image_id)) && (rename_order == RenameOrder::ADDING))
        {
            update_id += 1;
        }
        else if ((v.second > (image_id) || (image_id < static_cast<int32_t>(filteredImageDataID.size()) && v.second > image_id)) && (rename_order == RenameOrder::REMOVING))
        {
            update_id -= 1;
        }

        try
        {
            int32_t value = std::stoi(v.first);

            if ((value > (image_id + 1) || ((image_id + 1) < static_cast<int32_t>(filteredImageDataID.size()) && v.second > image_id)) && (rename_order == RenameOrder::ADDING))
            {
                value += 1;
            }
            else if ((v.second > (image_id) || (image_id < static_cast<int32_t>(filteredImageDataID.size()) && v.second > image_id)) && (rename_order == RenameOrder::REMOVING))
            {
                value -= 1;
            }

            update_key = std::to_string(value);
        }
        catch ([[maybe_unused]] const std::exception& e)
        {
            if (v.first.empty())
            {
                update_key = std::to_string(image_id + 1);
            }
            else
            {
                update_key = v.first;
            }
        }

        return std::make_pair(update_key, update_id);
    });

    return tmp;
}

std::unordered_map<int32_t, int32_t> FImage::RemapRepeatIDs(const std::unordered_map<int32_t, int32_t> &source, int32_t image_id, RenameOrder rename_order)
{
    decltype(repeatFilteredImageMap) tmp;
    std::ranges::transform(source, std::inserter(tmp, tmp.begin()) , [&](const std::pair<int32_t, int32_t> & v) {

        int32_t update_id = v.second;

        if ((v.second > (image_id + 1) || ((image_id + 1) < static_cast<int32_t>(repeatFilteredImageMap.size()) && v.second > image_id)) && (rename_order == RenameOrder::ADDING))
        {
            update_id += 1;
        }
        else if ((v.second > (image_id) || (image_id < static_cast<int32_t>(repeatFilteredImageMap.size()) && v.second > image_id)) && (rename_order == RenameOrder::REMOVING))
        {
            update_id -= 1;
        }

        int32_t update_key = v.first;

        if ((update_key > (image_id + 1) || ((image_id + 1) < static_cast<int32_t>(repeatFilteredImageMap.size()) && v.second > image_id)) && (rename_order == RenameOrder::ADDING))
        {
            update_key += 1;
        }
        else if ((v.second > (image_id) || (image_id < static_cast<int32_t>(repeatFilteredImageMap.size()) && v.second > image_id)) && (rename_order == RenameOrder::REMOVING))
        {
            update_key -= 1;
        }

        return std::make_pair(update_key, update_id);
    });

    return tmp;
}

int32_t FImage::privProcessFilter(const filter::types::FilterType &filter, const bool& save_filter, const int32_t& repeat)
{
    const int32_t image_id = currentViewLayer;
    const int32_t valid_image_id = privProcessFilter(image_id, filter, save_filter, repeat);
    return valid_image_id;
}

int32_t FImage::privProcessFilter(const int32_t& image_id, const filter::types::FilterType& filter, const bool& save_filter, const int32_t& repeat)
{
    std::lock_guard lock(mutex);

    int32_t valid_image_id = -1;

    const SDL_Surface* read_image = nullptr;
    const SDL_Surface* write_image = nullptr;
    const int32_t prev_view_layer = currentViewLayer;

    if (save_filter)
    {
        currentViewLayer = image_id + 1;
        valid_image_id = currentViewLayer;

        // create copy of current image

        const auto image = filteredImageData[image_id];

        SDL_Surface* copy = SDL_ConvertSurface(image.get(), image->format);
        filteredImageData.insert(filteredImageData.begin() + image_id + 1, std::shared_ptr<SDL_Surface>(copy, &FImage::privDeleteSurface));
        write_image = copy;

        filteredImageDataID.insert({"", image_id + 1});
        filterProcessCalls.insert(filterProcessCalls.begin() + image_id + 1, filter);

        if (repeat > 1)
        {
            repeatFilteredImageMap.insert({image_id + 1, repeat});
        }

        // create tmp buffer if there is more than 1 image filter

        if (!tmpBuffer && filteredImageData.size() > 1)
        {
            std::thread([this]() {
                std::lock_guard buf_lock(createBufferMutex);
                tmpBuffer = std::shared_ptr<SDL_Surface>(SDL_ConvertSurface(originalImage.get(), originalImage->format), &FImage::privDeleteSurface);
            }).detach();
        }

        // update mapped indexes

        filteredImageDataID = RemapIDs(filteredImageDataID, image_id, RenameOrder::ADDING);
        repeatFilteredImageMap = RemapRepeatIDs(repeatFilteredImageMap, image_id, RenameOrder::ADDING);
    }
    else
    {
        if (repeatFilteredImageMap.contains(image_id))
        {
            if (repeat > 1)
            {
                repeatFilteredImageMap[image_id] = repeat;
            }
            else if (repeat == 0)
            {
                repeatFilteredImageMap.erase(image_id);
            }
        }
        else
        {
            if (repeat > 1)
            {
                repeatFilteredImageMap.insert({image_id, repeat});
            }
        }


        if (image_id >= 0 && image_id < static_cast<int32_t>(filteredImageData.size()))
        {
            filterProcessCalls[image_id] = filter;
            write_image = filteredImageData[image_id].get();
            valid_image_id = image_id;
        }
        else
        {
            logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Not a valid image id"));
        }
    }

    read_image = privSelectReadImage(image_id, save_filter);

    if (read_image && write_image)
    {
        if (inProgressOfUpdatingFilters)
        {
            inProgressOfUpdatingFilters = false;

            std::unique_lock reapplyFilterLock(reapplyFilterMutex);
            cvWaitForThreadToComplete.wait(reapplyFilterLock, [this] {return isProgressThreadComplete;});
        }

        const uint8_t bytes_per_pixel = SDL_GetPixelFormatDetails(read_image->format)->bytes_per_pixel;
        const int32_t reapply_filter_offset_count = shouldReapplyFilters ? static_cast<int32_t>(filteredImageData.size()) - (image_id + 1) : 1;

        const SDL_Surface* read_image_tmp = privSelectReadImage(image_id, save_filter);
        SDL_Surface* write_image_tmp = save_filter ? filteredImageData[image_id + 1].get() : filteredImageData[image_id].get();
        const auto use_filter = save_filter ? filterProcessCalls[image_id + 1] : filterProcessCalls[image_id];

        if (read_image_tmp)
        {
            constexpr bool in_progress = true;
            const int32_t n_repeats = GetRepeatCount(image_id, save_filter);
            privRunFilter(read_image_tmp, write_image_tmp, use_filter, n_repeats, in_progress);

            // add thread job here

            if (reapply_filter_offset_count > 0)
            {
                inProgressOfUpdatingFilters = true;
                isProgressThreadComplete = false;
                threadPool.addjob([this, reapply_filter_offset_count=reapply_filter_offset_count, image_id=image_id, save_filter=save_filter, bytes_per_pixel=bytes_per_pixel, current_view_layer=currentViewLayer]() -> void {

                    std::lock_guard buf_lock(createBufferMutex);

                    for (int32_t i=1; i < reapply_filter_offset_count; i++)
                    {
                        const SDL_Surface* tp_read_image_tmp = privSelectReadImage(image_id + i, save_filter);
                        const SDL_Surface* tp_write_image_tmp = filteredImageData[image_id + i + 1].get();
                        auto tp_use_filter = filterProcessCalls[image_id + i + 1];

                        const int32_t reapply_thread_n_repeats = GetRepeatCount(image_id, save_filter);
                        privRunFilter(tp_read_image_tmp, tmpBuffer.get(), tp_use_filter, reapply_thread_n_repeats, inProgressOfUpdatingFilters);

                        if (!inProgressOfUpdatingFilters)
                        {
                            isProgressThreadComplete = true;
                            break;
                        }

                        const auto tmp_byte_data = static_cast<uint8_t*>(tmpBuffer->pixels);
                        const auto image_write_byte_data = static_cast<uint8_t*>(tp_write_image_tmp->pixels);
                        utility::fast_memcpy(image_write_byte_data, tmp_byte_data, bytes_per_pixel * tp_read_image_tmp->h);

                        if (current_view_layer != currentViewLayer && currentViewLayer != 0 && currentViewLayer > image_id)
                        {
                            hasViewChangedToInProcessFilter = true;
                        }
                    }

                    inProgressOfUpdatingFilters = false;
                    isProgressThreadComplete = true;
                    cvWaitForThreadToComplete.notify_all();
                });
            }
        }
        else
        {
            logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "invalid image id filter"));
        }

        if (prev_view_layer != currentViewLayer && save_filter)
        {
            texture.Update(static_cast<uint8_t*>(write_image->pixels), write_image->w, write_image->h);
        }
    }
    else
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "No data to apply filter!"));
    }

    return valid_image_id;
}

int32_t FImage::privProcessFilter(const std::string& filter_name, const filter::types::FilterType& filter, const bool& save_filter, const int32_t& repeat)
{
    int32_t valid_image_id = -1;

    try
    {
        const int32_t image_id = filteredImageDataID.at(filter_name);
        valid_image_id = ProcessFilter(image_id, filter, save_filter, repeat);
    }
    catch (const std::exception& e)
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Unable to find filtered image, exception msg: {}", e.what()));
    }

    return valid_image_id;
}

int32_t FImage::privProcessFilter(const std::string& filter_name, const std::string& filter_name_to, const filter::types::FilterType& filter, const int32_t& repeat)
{
    int32_t valid_image_id = -1;
    int32_t image_id = -1;

    try
    {
        image_id = filteredImageDataID.at(filter_name);
    }
    catch (const std::exception& e)
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Unable to find filtered image, exception msg: {}", e.what()));
    }

    const bool existing_image_id = filteredImageDataID.contains(filter_name_to);
    if (existing_image_id)
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "New filter name must be unique, {} already exists!", filter_name_to));
    }

    if (image_id >= 0 && !existing_image_id)
    {
        constexpr bool save_filter = true;
        valid_image_id = privProcessFilter(image_id, filter, save_filter, repeat);
        ChangeFilterName(valid_image_id, filter_name_to);
    }
    else
    {
        logging::warn(fmt::format(fmt::fg(fmt::terminal_color::bright_yellow), "Unable to apply filtered image"));
    }

    return valid_image_id;
}

int32_t FImage::GetRepeatCount(const int32_t image_id, const bool save_filter)
{
    int32_t n_repeats;

    if (save_filter && repeatFilteredImageMap.contains(image_id + 1))
    {
        n_repeats = repeatFilteredImageMap[image_id + 1];
    }
    else if (!save_filter && repeatFilteredImageMap.contains(image_id))
    {
        n_repeats = repeatFilteredImageMap[image_id];
    }
    else
    {
        n_repeats = 1;
    }

    return n_repeats;
}

SDL_Surface * FImage::privRunFilter(const SDL_Surface *read_image, SDL_Surface *write_image, const filter::types::FilterType &filter, const int32_t repeat_filter_n_times, const bool& in_progress)
{
    const uint8_t bytes_per_pixel = SDL_GetPixelFormatDetails(read_image->format)->bytes_per_pixel;

    for (int32_t r=0; r < repeat_filter_n_times; r++)
    {
        if (!in_progress)
        {
            isProgressThreadComplete = true;
            break;
        }

        if (r > 0)
        {
            read_image = SDL_ConvertSurface(write_image, write_image->format);
        }

        const auto byte_read_image = static_cast<uint8_t *>(read_image->pixels);
        const std::span image_data_view(byte_read_image, read_image->w * read_image->h * bytes_per_pixel);
        auto image_data_pixels = image_data_view | gss::views::stride(bytes_per_pixel) | gss::views::transform([](uint8_t& value) -> uint8_t* { return &value;});

        gss::cprocess::loops::for_each(std::get<filter::types::ExecutionPolicies>(filter), image_data_pixels.begin(), image_data_pixels.end(), [&]([[maybe_unused]] const uint8_t * pixel_ptr) -> void {
                /**
                 * @summary
                 * will do a parallel/sequence execution on the image data to appy filter. Wether this is parallel or
                 * sequential execution depends on the filter being used
                 *
                 * @brief pixel_ptr
                 * is the pointing to the start of an individual pixel
                 * m_data is all the pixels and the respective channels like r,g,b
                 *
                 * @brief idx
                 * is the actual pixel index which can be found by subtracting the address of d from the actual
                 * address of where the pixel data is. Since the original pixel data see pixels as the individual channel
                 * the need to divide by the bytes per pixel (bpp) is needed to make sure we have the correct index that
                 * corresponds to the number of pixels and not the channels
                 *
                 * @brief The filter (FilterType)
                 * is setup to expect the original data source and the current row and column depnding on
                 * pixel of the image
                 *
                 * @more info
                 * need to pass in the bytes per pixel, image width/height and pitch (the number of bytes per row)
                 */

                if (!in_progress) return;

                const auto idx = static_cast<uint32_t>(pixel_ptr - reinterpret_cast<uint8_t *>(image_data_pixels.front())) / bytes_per_pixel;
                const int32_t j = idx % read_image->w;
                const int32_t i = idx / read_image->w;

                const auto pixel_value = std::get<filter::types::FilterFuncType>(filter)(image_data_pixels.front(), j, i, bytes_per_pixel, read_image->w, read_image->h, read_image->pitch, std::get<filter::types::FilterUserTypes>(filter));

                const auto image_write_byte_data = static_cast<uint8_t*>(write_image->pixels);
                image_write_byte_data[(j * bytes_per_pixel) + (i * write_image->pitch) + 0] = std::get<0>(pixel_value);
                image_write_byte_data[(j * bytes_per_pixel) + (i * write_image->pitch) + 1] = std::get<1>(pixel_value);
                image_write_byte_data[(j * bytes_per_pixel) + (i * write_image->pitch) + 2] = std::get<2>(pixel_value);

                if (bytes_per_pixel == 4)
                {
                    image_write_byte_data[(j * bytes_per_pixel) + (i * write_image->pitch) + 3] = std::get<3>(pixel_value);
                }
        });
    }

    return write_image;
}

SDL_Surface* FImage::privSelectReadImage(const int32_t& image_id, const bool& save_filter) const
{
    SDL_Surface* read_image = nullptr;

    if (!filteredImageData.empty() && image_id >= 0 && image_id < static_cast<int32_t>(filteredImageData.size()))
    {
        if (save_filter)
        {
            read_image = filteredImageData[image_id].get();
        }
        else
        {
            if (image_id == 0)
            {
                read_image = originalImage.get();
            }
            else
            {
                read_image = filteredImageData[image_id-1].get();
            }
        }
    }

    return read_image;
}

void FImage::privDeleteSurface(SDL_Surface *surface)
{
    SDL_DestroySurface(surface);
}

cthreadpool FImage::threadPool = cthreadpool(MAX_ENHANCE_SPRITE_THREADS, "TP_EhnceSprt");
