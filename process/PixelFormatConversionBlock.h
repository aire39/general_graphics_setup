#pragma once

#include "ImageProcessBlock.h"

class PixelFormatConversionBlock final : public ImageProcessBlock
{
  public:
    PixelFormatConversionBlock() = default;
    explicit PixelFormatConversionBlock(const std::string &thread_name, const std::string &thread_description);
    explicit PixelFormatConversionBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others);
  protected:
    std::vector<std::shared_ptr<FImage>> Process(std::vector<std::shared_ptr<FImage>> image_sources) override;
};
