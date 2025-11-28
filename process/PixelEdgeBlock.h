#pragma once

#include "ImageProcessBlock.h"

class PixelEdgeBlock final : public ImageProcessBlock
{
  public:
    PixelEdgeBlock() = default;
    explicit PixelEdgeBlock(const std::string &thread_name, const std::string &thread_description);
    explicit PixelEdgeBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others);

    float TimeToComplete() const override { return timeToComplete; }
    float TimeToCompleteFilter() const override { return timeFilterToComplete; }

  protected:
    std::vector<std::shared_ptr<FImage>> Process(std::vector<std::shared_ptr<FImage>> image_sources, DataContainer& data_sources) override;
};
