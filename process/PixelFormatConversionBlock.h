#pragma once

#include <mutex>
#include "ImageProcessBlock.h"
#include "video/camera/common/CameraTypes.h"
#include "video/camera/common/CameraFormats.h"

class PixelFormatConversionBlock final : public ImageProcessBlock
{
  public:
    PixelFormatConversionBlock() = default;
    explicit PixelFormatConversionBlock(const std::string &thread_name, const std::string &thread_description);
    explicit PixelFormatConversionBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others);

    void SetFormatConversionType(gss::video::camera::types::VideoFormat video_format);
    gss::video::camera::types::VideoFormat GetFormatConversionType();

    protected:
      std::vector<std::shared_ptr<FImage>> Process(std::vector<std::shared_ptr<FImage>> image_sources) override;

    private:
      std::mutex mtxChangeFormat;
      gss::video::camera::types::VideoFormat videoFormat = gss::video::camera::formats::UYVY_FORMAT;
};
