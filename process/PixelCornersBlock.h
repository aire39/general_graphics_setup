#pragma once

#include "ImageProcessBlock.h"

class PixelCornersBlock : public ImageProcessBlock
{
  public:
    PixelCornersBlock() = default;
    explicit PixelCornersBlock(const std::string &thread_name, const std::string &thread_description);
    explicit PixelCornersBlock(const std::string &thread_name, const std::string &thread_description, std::shared_ptr<ImageProcessBlock> other);

  protected:
    std::vector<std::shared_ptr<FImage>> Process(std::vector<std::shared_ptr<FImage>> image_sources) override;
};
