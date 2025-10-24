#pragma once

#include "ImageProcessBlock.h"

class PixelCornersBlock : public ImageProcessBlock
{
  public:
    PixelCornersBlock() = default;
    explicit PixelCornersBlock(const std::string &thread_name, const std::string &thread_description);
    explicit PixelCornersBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others);

    void SetSigma(double sigma);
    void SetKValue(float k);

  protected:
    std::vector<std::shared_ptr<FImage>> Process(std::vector<std::shared_ptr<FImage>> image_sources) override;

  private:
    double sigmaFactor = 1.0;
    float kFactor = 0.04f;
};
