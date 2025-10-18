#include "ImageProcessBlock.h"

#include "graphics/images/FImage.h"

namespace {
  constexpr uint32_t default_waitfor_queue_timeout = 100u;
}

ImageProcessBlock::ImageProcessBlock(const std::string &thread_name, const std::string &thread_description)
{
  name = thread_name;
  description = thread_description;
}

ImageProcessBlock::ImageProcessBlock(const std::string &thread_name, const std::string &thread_description, std::shared_ptr<ImageProcessBlock> other)
{
  name = thread_name;
  description = thread_description;
  connection = other;
}

ImageProcessBlock::~ImageProcessBlock()
{
  enableProcess = false;
  if (processThread.joinable())
  {
    processThread.join();
  }
}

void ImageProcessBlock::QueueToProcess(std::shared_ptr<FImage> image)
{
  if (enableProcess && image)
  {
    {
      std::lock_guard q_lock(mtxImageQueue);
      imageQueue.push(image);
    }

    std::unique_lock lock(mtxCVImageQueue);
    lock.unlock();

    cvImageQueue.notify_one();
  }
}

void ImageProcessBlock::Enable(const bool enable)
{
  enableProcess = enable;

  if (enableProcess)
  {
    processThread = cthread(name.c_str(), description.c_str(), &ImageProcessBlock::RunProcessTask, this);
  }
}

ImageProcessBlock::DataFlow ImageProcessBlock::GetDataFlow() const
{
  return dataFlow;
}

std::shared_ptr<FImage> ImageProcessBlock::GetImage()
{
  std::shared_ptr<FImage> image = nullptr;

  {
    std::lock_guard q_lock(mtxImageOutQueue);
    if (!imageOutQueue.empty())
    {
      image = imageOutQueue.front();
      imageOutQueue.pop();
    }
  }

  return image;
}

void ImageProcessBlock::RunProcessTask()
{
  while (enableProcess)
  {
    {
      std::unique_lock lock(mtxCVImageQueue);
      cvImageQueue.wait_for(lock, std::chrono::milliseconds(default_waitfor_queue_timeout), [this]() {
        return !imageQueue.empty();
      });
    }

    std::shared_ptr<FImage> source_image = nullptr;
    {
      std::lock_guard q_lock(mtxImageQueue);
      if (!imageQueue.empty())
      {
        source_image = imageQueue.front();
        imageQueue.pop();
      }
    }

    std::shared_ptr<FImage> output_image = nullptr;
    if (source_image)
    {
      output_image = Process(source_image);
    }

    if ((dataFlow == DataFlow::F_OUT || dataFlow == DataFlow::F_INOUT) && output_image)
    {
      std::lock_guard q_lock(mtxImageOutQueue);
      imageOutQueue.push(output_image);
    }

    if (connection && output_image)
    {
      connection->QueueToProcess(output_image);
    }
  }
}
