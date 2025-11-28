#pragma once

#include <cstdint>
#include <vector>
#include "graphics/images/FImage.h"

class ImageProcessBlock;

class ImageBlockViewMenu
{
  public:
    ImageBlockViewMenu() = delete;
    explicit ImageBlockViewMenu(const std::vector<ImageProcessBlock*> &image_blocks);
    ~ImageBlockViewMenu() = default;

    void RenderMenu();
    std::shared_ptr<FImage> GetImage() const;

  private:
    std::vector<ImageProcessBlock*> imageBlocks;
    int32_t viewIndex = 0;
};
