#pragma once

#include <queue>
#include <utility>
#include <condition_variable>
#include <mutex>

#include "opencv2/opencv.hpp"
#include "ImageProcessBlock.h"
#include "common/cthreads/cthread.h"

class PixelCornersBlock : public ImageProcessBlock
{
  public:
    PixelCornersBlock() = default;
    explicit PixelCornersBlock(const std::string &thread_name, const std::string &thread_description);
    explicit PixelCornersBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others);
    ~PixelCornersBlock();

    void SetSigma(double sigma);
    void SetKValue(float k);

    float TimeToComplete() const;
    float TimeToCompleteFilter() const;

  protected:
    std::vector<std::shared_ptr<FImage>> Process(std::vector<std::shared_ptr<FImage>> image_sources) override;
    void ExtraEnableProcess() override;

  private:
    float timeToComplete = 0.0f; // milliseconds
    float timeFilterToComplete = 0.0f; // milliseconds
    double sigmaFactor = 1.0;
    float kFactor = 0.04f;

    std::queue<std::pair<std::reference_wrapper<cv::Mat>, std::reference_wrapper<cv::Mat>>> sourcesGXQueue;
    std::queue<std::pair<std::reference_wrapper<cv::Mat>, std::reference_wrapper<cv::Mat>>> sourcesGYQueue;
    cthread produceGradXThread;
    cthread produceGradYThread;

    bool gradXComplete = false;
    bool gradYComplete = false;
    std::mutex mtxGradX;
    std::mutex mtxGradY;
    std::mutex mtxReceivedInput;
    std::condition_variable produceGradXCondition;
    std::condition_variable produceGradYCondition;
    std::condition_variable receivedInputCondition;

    void GradXTask();
    void GradYTask();
};
