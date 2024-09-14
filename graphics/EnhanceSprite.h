#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <mutex>
#include "Sprite.h"
#include "Filters.h"

class SDL_Surface;

class EnhanceSprite final : public Sprite
{
  public:
    EnhanceSprite() = default;
    ~EnhanceSprite() override = default;

    void LoadTexture(const std::string& image_file) override;
    void LoadTexture(const SDL_Surface* image) override;

    void ProcessFilter(const filter::types::FilterType& filter, bool save_filter);
    void ProcessFilter(const filter::types::FilterType& filter, int32_t image_id, bool save_filter);
    void ProcessFilter(const filter::types::FilterType& filter, const std::string& filter_name, bool save_filter);

    void ChangeFilterName(const std::string& filter_name, const std::string& new_filter_name);
    void ChangeFilterName(int32_t filter_id, const std::string& new_filter_name);

    void ViewTexture(uint32_t filter_id) const;
    void ViewTexture(const std::string& filter_name) const;

    void ResetFilters();

  private:
    SDL_Surface* originalImage = nullptr;
    SDL_Surface* baseImage = nullptr;
    std::vector<SDL_Surface*> filteredImageData;
    std::vector<filter::types::FilterType> filterProcessCalls;
    std::unordered_map<std::string, int32_t> filteredImageDataID;
    std::mutex mutex;
};
