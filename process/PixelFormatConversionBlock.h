#pragma once

#include "ImageProcessBlock.h"

class PixelFormatConversionBlock final : public ImageProcessBlock
{
  public:
    PixelFormatConversionBlock() = default;
    explicit PixelFormatConversionBlock(const std::string &thread_name, const std::string &thread_description);
    explicit PixelFormatConversionBlock(const std::string &thread_name, const std::string &thread_description, std::shared_ptr<ImageProcessBlock> other);
  protected:
    std::vector<std::shared_ptr<FImage>> Process(std::vector<std::shared_ptr<FImage>> image_sources) override;
};
