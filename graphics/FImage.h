/*!
 *
 * FImage class
 *
 * summary: This is meant to easily process images with custom functions creating convolutions or other small functions
 * that work opn the pixel data set for this class
 *
 * FImage is short for FilterImage
 *
 */

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <atomic>
#include <glm/glm.hpp>
#include <SDL3/SDL_pixels.h>

#include "Sprite.h"
#include "FilterTypes.h"
#include "cthreads/cthreadpool.h"

struct SDL_Surface;

class FImage final : public Sprite
{
  public:
    FImage();
    explicit FImage(const std::string& new_name);
    explicit FImage(const std::string& new_name, int32_t width, int32_t height, SDL_PixelFormat format);
    ~FImage() override;

    void LoadTexture(const std::string& image_file) override;
    void LoadTexture(const SDL_Surface* image) override;

    bool ProcessFilter(const filter::types::FilterType& filter, bool save_filter, int32_t repeat = -1);
    bool ProcessFilter(int32_t image_id, const filter::types::FilterType& filter, bool save_filter, int32_t repeat = -1);
    bool ProcessFilter(const std::string& filter_name, const filter::types::FilterType& filter, bool save_filter, int32_t repeat = -1);
    bool ProcessFilter(const std::string& filter_name, const std::string& filter_name_to, const filter::types::FilterType& filter, int32_t repeat = -1);

    void SetPixel(int32_t image_id, int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) const;
    void SetPixel(int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) const;
    void SetPixel(int32_t image_id, glm::ivec2 pos, glm::u8vec4 color) const;
    void SetPixel(glm::ivec2 pos, glm::u8vec4 color) const;
    [[nodiscard]] glm::u8vec4 GetPixel(int32_t image_id, int32_t x, int32_t y) const;
    [[nodiscard]] glm::u8vec4 GetPixel(int32_t x, int32_t y) const;
    [[nodiscard]] uint8_t GetRedPixel(int32_t x, int32_t y) const;
    [[nodiscard]] uint8_t GetRedPixel(int32_t image_id, int32_t x, int32_t y) const;
    [[nodiscard]] uint8_t GetGreenPixel(int32_t x, int32_t y) const;
    [[nodiscard]] uint8_t GetGreenPixel(int32_t image_id, int32_t x, int32_t y) const;
    [[nodiscard]] uint8_t GetBluePixel(int32_t x, int32_t y) const;
    [[nodiscard]] uint8_t GetBluePixel(int32_t image_id, int32_t x, int32_t y) const;
    [[nodiscard]] uint8_t GetAlphaPixel(int32_t x, int32_t y) const;

    void ChangeFilterName(const std::string& filter_name, const std::string& new_filter_name);
    void ChangeFilterName(int32_t filter_id, const std::string& new_filter_name);

    void ViewTexture();
    void ViewTexture(int32_t filter_id);
    void ViewTexture(const std::string& filter_name);

    [[nodiscard]] int32_t GetBaseImageFilterID() const;
    [[nodiscard]] int32_t GetCurrentImageFilterID() const;
    [[nodiscard]] int32_t GetFinalImageFilterID() const;

    [[nodiscard]] std::vector<std::pair<std::string, int32_t>> GetProcessedImageNames() const;
    [[nodiscard]] std::string GetProcessedImageNameByID(int32_t image_id) const;

    int32_t GetWidth() const override;
    int32_t GetHeight() const override;
    int32_t GetBytesPerPixel() const override;

    void SetShouldReapplyFilters(bool enable);

    void Remove();
    void Remove(int32_t image_id);
    void Remove(const std::string& image_name);
    [[nodiscard]] SDL_Surface* Extract();
    [[nodiscard]] SDL_Surface* Extract(int32_t image_id);
    [[nodiscard]] SDL_Surface* Extract(const std::string& image_name);

    void ResetFilters();

    void Draw() override;

  private:
    int32_t currentViewLayer = 0;
    SDL_Surface* originalImage = nullptr;
    SDL_Surface* tmpBuffer = nullptr;
    bool hasViewChangedToInProcessFilter = false;

    std::vector<SDL_Surface*> filteredImageData;
    std::vector<filter::types::FilterType> filterProcessCalls;
    std::unordered_map<std::string, int32_t> filteredImageDataID;
    std::unordered_map<int32_t, int32_t> repeatFilteredImageMap;

    bool shouldReapplyFilters = true;
    bool inProgressOfUpdatingFilters = false;
    bool isProgressThreadComplete = true;

    std::condition_variable cvWaitForThreadToComplete;
    std::mutex reapplyFilterMutex;
    std::mutex createBufferMutex;
    std::mutex updateTextureMutex;
    std::mutex mutex;

    void ResetSprite();

    enum class RenameOrder {ADDING, REMOVING};
    std::unordered_map<std::string, int32_t> RemapIDs(const std::unordered_map<std::string, int32_t>& source, int32_t image_id, RenameOrder rename_order);
    std::unordered_map<int32_t, int32_t> RemapRepeatIDs(const std::unordered_map<int32_t, int32_t>& source, int32_t image_id, RenameOrder rename_order);

    int32_t privProcessFilter(const filter::types::FilterType& filter, const bool& save_filter, const int32_t& repeat);
    int32_t privProcessFilter(const int32_t& image_id, const filter::types::FilterType& filter, const bool& save_filter, const int32_t& repeat);
    int32_t privProcessFilter(const std::string& filter_name, const filter::types::FilterType &filter, const bool& save_filter, const int32_t& repeat);
    int32_t privProcessFilter(const std::string& filter_name, const std::string& filter_name_to, const filter::types::FilterType& filter, const int32_t& repeat);

    int32_t GetRepeatCount(int32_t image_id, bool save_filter);
    SDL_Surface* privRunFilter(const SDL_Surface* read_image, SDL_Surface* write_image, const filter::types::FilterType &filter, int32_t repeat_filter_n_times, const bool& in_progress);

    SDL_Surface* privSelectReadImage(const int32_t& image_id, const bool& save_filter) const;

    static cthreadpool threadPool;
};
