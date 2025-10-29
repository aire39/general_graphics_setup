#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include "ImageProcessBlock.h"
#include "opencv2/opencv.hpp"

class NonMaximumSuppressBlock final : public ImageProcessBlock
{
  public:
    NonMaximumSuppressBlock();
    explicit NonMaximumSuppressBlock(const std::string &thread_name, const std::string &thread_description);
    explicit NonMaximumSuppressBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others);
    ~NonMaximumSuppressBlock() override;

    void SetResponseFactor(double response_factor);
    void SetMinDistanceThreshold(double min_distance);
    void SetPointSize(double point_size);
    void SetPointColor(std::array<double, 3> color);

    float TimeToComplete() const override { return timeToComplete; }
    float TimeToCompleteFilter() const override { return timeFilterToComplete; }

    uint32_t GetNumberOfKeypoints() const;

  protected:
    std::vector<std::shared_ptr<FImage>> Process(std::vector<std::shared_ptr<FImage>> image_sources, DataContainer& data_sources) override;
    void ExtraEnableProcess() override;
    void ExtraDisableProcess() override;

  private:
    float timeToComplete = 0.0f; // milliseconds
    float timeFilterToComplete = 0.0f; // milliseconds

    double responseThresholdFactor = 0.1;
    double minDistanceThreshold = 20.0;
    double diameterThreshold = 4.0;
    std::array<double, 3> pointColor = { { 1.0, 1.0, 1.0 } };

    uint32_t numberOfKeypoints = 0;
    std::vector<cv::KeyPoint> finalKeyPoints;
};
