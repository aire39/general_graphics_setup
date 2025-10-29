#pragma once

#include <memory>
#include "ImageProcessBlock.h"
#include <opencv2/opencv.hpp>

class FImage;

class OpticalFlowBlock final : public ImageProcessBlock
{
  public:
    OpticalFlowBlock() = default;
    explicit OpticalFlowBlock(const std::string &thread_name, const std::string &thread_description);
    explicit OpticalFlowBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others);

    float TimeToComplete() const override { return timeToComplete; }
    float TimeToCompleteFilter() const override { return timeFilterToComplete; }

    void SetMaxFlowSteps(size_t max_flow_steps);
    void SetMinPoints(size_t min_points);

    uint32_t GetNumberOfGoodTracks() const;

  protected:
    std::vector<std::shared_ptr<FImage>> Process(std::vector<std::shared_ptr<FImage>> image_sources, DataContainer& data_sources) override;

  private:
    std::shared_ptr<FImage> lastGradXFimage = nullptr;
    std::shared_ptr<FImage> lastGradYFimage = nullptr;
    std::shared_ptr<FImage> lastGrayImage = nullptr;
    size_t maxflowSteps = 50;
    size_t trackPointMinThreshold = 50;
    [[maybe_unused]] size_t trackPointMaxThreshold = 100;
    std::vector<cv::KeyPoint> lastTrackPoints;
    std::vector<cv::KeyPoint> lastGoodTrackPoints;
    std::vector<cv::KeyPoint> goodTrackPoints;
};
