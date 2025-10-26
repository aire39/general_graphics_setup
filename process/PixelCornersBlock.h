#pragma once

#include <queue>
#include <utility>
#include <tuple>
#include <condition_variable>
#include <barrier>
#include <mutex>

#include "ImageProcessBlock.h"
#include "common/cthreads/cthread.h"
#include "opencv2/opencv.hpp"

class PixelCornersBlock final : public ImageProcessBlock
{
  public:
    PixelCornersBlock();
    explicit PixelCornersBlock(const std::string &thread_name, const std::string &thread_description);
    explicit PixelCornersBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others);
    ~PixelCornersBlock() override;

    void SetSigma(double sigma);
    void SetKValue(float k);

    float TimeToComplete() const;
    float TimeToCompleteFilter() const;

  protected:
    std::vector<std::shared_ptr<FImage>> Process(std::vector<std::shared_ptr<FImage>> image_sources) override;
    void ExtraEnableProcess() override;
    void ExtraDisableProcess() override;

  private:
    float timeToComplete = 0.0f; // milliseconds
    float timeFilterToComplete = 0.0f; // milliseconds
    double sigmaFactor = 1.0;
    float kFactor = 0.04f;

    bool gradComplete = false;
    bool gradBlurComplete = false;

    std::queue<std::pair<std::reference_wrapper<cv::Mat>, std::reference_wrapper<cv::Mat>>> sourcesGXQueue;
    std::queue<std::pair<std::reference_wrapper<cv::Mat>, std::reference_wrapper<cv::Mat>>> sourcesGYQueue;
    std::queue<std::tuple<std::reference_wrapper<cv::Mat>, std::reference_wrapper<cv::Mat>, std::reference_wrapper<cv::Mat>>> sourcesGXYQueue;
    std::queue<std::tuple<std::reference_wrapper<cv::Mat>, std::reference_wrapper<cv::Mat>, std::reference_wrapper<cv::Mat>>> sourcesGXXQueue;
    std::queue<std::tuple<std::reference_wrapper<cv::Mat>, std::reference_wrapper<cv::Mat>, std::reference_wrapper<cv::Mat>>> sourcesGYYQueue;
    cthread produceGradXThread;
    cthread produceGradYThread;
    cthread produceGradXYThread;
    cthread produceGradXXThread;
    cthread produceGradYYThread;

    std::mutex mtxReceivedInput;
    std::mutex mtxReceivedGrad;
    std::mutex mtxGrad;
    std::mutex mtxGradBlur;
    std::condition_variable receivedInputCondition;
    std::condition_variable receivedGradCondition;
    std::condition_variable produceGradCondition;
    std::condition_variable produceGradBlurCondition;

    class on_completion_grad {
        PixelCornersBlock * ppcb;

        public:
          explicit on_completion_grad(PixelCornersBlock * pcb) : ppcb(pcb) {}
          void operator()() const noexcept { ppcb->OnPhaseGradCompletion(); }
    };

    class on_completion_grad_blur {
        PixelCornersBlock * ppcb;

        public:
          explicit on_completion_grad_blur(PixelCornersBlock * pcb) : ppcb(pcb) {}
          void operator()() const noexcept { ppcb->OnPhaseGradBlurCompletion(); }
    };

    std::barrier<on_completion_grad> gradSync;
    std::barrier<on_completion_grad_blur> gradBlurSync;

    void GradXTask();
    void GradYTask();
    void GradXYTask();
    void GradXXTask();
    void GradYYTask();

    void OnPhaseGradCompletion() noexcept;
    void OnPhaseGradBlurCompletion() noexcept;
};
