#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <utility>
#include "graphics/Filters.h"

class FImage;
namespace gss::video::camera{class CameraBase;}

class CameraMenu
{
  public:
    CameraMenu() = delete;
    explicit CameraMenu(gss::video::camera::CameraBase* camera);
    ~CameraMenu() = default;

    void UpdateFrame(FImage* image) const;
    void RenderMenu();

private:
    enum class MenuOptions {INFO, SETTINGS, FILTERS};

    MenuOptions option = MenuOptions::INFO;
    gss::video::camera::CameraBase* camera;
    int32_t frameRate[2] {};
    int32_t resolution[2] {};
    int32_t videoFormat = 0;
    bool ignoreFormatFail = true;
    int32_t selectedFilterIndex = 0;

    filter::types::FilterType selectedFilter = filter::functions::cpu::parallel_vectorize::default_filter_process;
    typedef std::array<std::pair<std::string, filter::types::FilterType>, 6> FilterList;
    FilterList filters = {};

    static bool FilterListGetter(void* filter_ptr, int32_t index, const char ** text);
};
