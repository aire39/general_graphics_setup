#pragma once

#include <cstdint>
#include <queue>
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
    explicit ImageProcessBlock(const std::string &thread_name, const std::string &thread_description, std::shared_ptr<ImageProcessBlock> other);
    virtual ~ImageProcessBlock();

    void QueueToProcess(std::shared_ptr<FImage> image);
    void Enable(bool enable);

    DataFlow GetDataFlow() const;
    std::shared_ptr<FImage> GetImage();

  protected:
    std::string name = "name";
    std::string description = "description";
    DataFlow dataFlow = DataFlow::F_IN;
    std::shared_ptr<ImageProcessBlock> connection = nullptr;
    std::queue<std::shared_ptr<FImage>> imageQueue;
    std::queue<std::shared_ptr<FImage>> imageOutQueue;
    cthread processThread;

    virtual std::shared_ptr<FImage> Process(std::shared_ptr<FImage> image_source) = 0;

  private:
    bool enableProcess = false;
    std::condition_variable cvImageQueue;
    std::mutex mtxCVImageQueue;
    std::mutex mtxImageQueue;
    std::mutex mtxImageOutQueue;
    void RunProcessTask();

};
