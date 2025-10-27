#pragma once

#include <cstdint>
#include <queue>
#include <vector>
#include <memory>
#include <string>
#include <condition_variable>
#include "../common/cthreads/cthread.h"

class FImage;

class ImageProcessBlock
{
  public:
    enum class DataFlow {F_IN, F_OUT, F_INOUT};

    ImageProcessBlock() = default;
    explicit ImageProcessBlock(const std::string &thread_name, const std::string &thread_description);
    explicit ImageProcessBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others);
    virtual ~ImageProcessBlock();

    void QueueToProcess(std::vector<std::shared_ptr<FImage>> images);
    void Enable(bool enable);

    virtual float TimeToComplete() const { return timeToComplete; } // shouldn't need to make this virtual. fix this later
    virtual float TimeToCompleteFilter() const { return timeFilterToComplete; } // shouldn't need to make this virtual. fix this later

    bool IsEnabled() const;

    DataFlow GetDataFlow() const;
    std::vector<std::shared_ptr<FImage>> GetImage();
    std::shared_ptr<FImage> GetLastImage();
    std::shared_ptr<FImage> GetFrontImage();

  protected:
    std::string name = "name";
    std::string description = "description";
    float timeToComplete = 0.0f; // milliseconds
    float timeFilterToComplete = 0.0f; // milliseconds
    DataFlow dataFlow = DataFlow::F_IN;
    std::vector<std::shared_ptr<ImageProcessBlock>> connections;
    std::queue<std::vector<std::shared_ptr<FImage>>> imageQueue;
    std::queue<std::vector<std::shared_ptr<FImage>>> imageOutQueue;
    cthread processThread;

    void SetMaxQueueSize(int32_t max_queue_size);
    virtual std::vector<std::shared_ptr<FImage>> Process(std::vector<std::shared_ptr<FImage>> image_sources) = 0;
    virtual void ExtraEnableProcess() {}
    virtual void ExtraDisableProcess() {}

  private:
    int32_t maxQueueSize = 1;
    bool enableProcess = false;
    std::condition_variable cvImageQueue;
    std::mutex mtxCVImageQueue;
    std::mutex mtxImageQueue;
    std::mutex mtxImageOutQueue;
    void RunProcessTask();

    static inline uint32_t droppedImages = 0;
};

typedef std::vector<std::shared_ptr<ImageProcessBlock>> ImageBlockList;