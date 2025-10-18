#pragma once

#include "ImageProcessBlock.h"

class PixelEdgeBlock final : public ImageProcessBlock
{
  public:
    PixelEdgeBlock() = default;
    explicit PixelEdgeBlock(const std::string &thread_name, const std::string &thread_description);
    explicit PixelEdgeBlock(const std::string &thread_name, const std::string &thread_description, std::shared_ptr<ImageProcessBlock> other);
  protected:
    std::shared_ptr<FImage> Process(std::shared_ptr<FImage> image_source) override;
};
