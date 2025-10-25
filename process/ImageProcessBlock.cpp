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

ImageProcessBlock::ImageProcessBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others)
{
  name = thread_name;
  description = thread_description;
  connections = others;
}

ImageProcessBlock::~ImageProcessBlock()
{
  enableProcess = false;
  if (processThread.joinable())
  {
    processThread.join();
  }
}

void ImageProcessBlock::QueueToProcess(std::vector<std::shared_ptr<FImage>> images)
{
  if (enableProcess && !images.empty())
  {
    {
      std::lock_guard q_lock(mtxImageQueue);
      imageQueue.push(images);
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
    ExtraEnableProcess();
    processThread = cthread(name.c_str(), description.c_str(), &ImageProcessBlock::RunProcessTask, this);
  }
}

bool ImageProcessBlock::IsEnabled() const
{
  return enableProcess;
}

ImageProcessBlock::DataFlow ImageProcessBlock::GetDataFlow() const
{
  return dataFlow;
}

std::vector<std::shared_ptr<FImage>> ImageProcessBlock::GetImage()
{
  std::vector<std::shared_ptr<FImage>> images;

  {
    std::lock_guard q_lock(mtxImageOutQueue);
    if (!imageOutQueue.empty())
    {
      images = imageOutQueue.front();
      imageOutQueue.pop();
    }
  }

  return images;
}

std::shared_ptr<FImage> ImageProcessBlock::GetLastImage()
{
  std::shared_ptr<FImage> image = nullptr;

  {
    std::lock_guard q_lock(mtxImageOutQueue);
    if (!imageOutQueue.empty())
    {
      const auto images = imageOutQueue.front();
      imageOutQueue.pop();

      image = images.back();
    }
  }

  return image;
}

std::shared_ptr<FImage> ImageProcessBlock::GetFrontImage()
{
  std::shared_ptr<FImage> image = nullptr;

  {
    std::lock_guard q_lock(mtxImageOutQueue);
    if (!imageOutQueue.empty())
    {
      const auto images = imageOutQueue.front();
      imageOutQueue.pop();

      image = images.front();
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

    std::vector<std::shared_ptr<FImage>> source_images;
    {
      std::lock_guard q_lock(mtxImageQueue);
      if (!imageQueue.empty())
      {
        source_images = imageQueue.front();
        imageQueue.pop();
      }
    }

    std::vector<std::shared_ptr<FImage>> output_image;
    if (!source_images.empty())
    {
      output_image = Process(source_images);
    }

    if ((dataFlow == DataFlow::F_OUT || dataFlow == DataFlow::F_INOUT) && !output_image.empty())
    {
      std::lock_guard q_lock(mtxImageOutQueue);
      imageOutQueue.push(output_image);
    }

    if (!connections.empty() && !output_image.empty())
    {
      for (auto conn : connections)
      {
        conn->QueueToProcess(output_image);
      }
    }
  }
}
