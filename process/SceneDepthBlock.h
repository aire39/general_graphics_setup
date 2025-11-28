#pragma once

#include "ImageProcessBlock.h"
#include <opencv2/opencv.hpp>

class SceneDepthBlock : public ImageProcessBlock
{
  public:
    SceneDepthBlock() = default;
    explicit SceneDepthBlock(const std::string &thread_name, const std::string &thread_description);
    explicit SceneDepthBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others);

    void SetCameraRefPosition(std::array<float, 3> position);
    void SetCameraRefFocal(std::array<float, 2> focal);
    void SetCameraPosition(std::array<float, 3> position, int32_t index);
    void SetCameraFocal(std::array<float, 2> focal, int32_t index);

  protected:
    std::vector<std::shared_ptr<FImage>> Process(std::vector<std::shared_ptr<FImage>> image_sources, DataContainer &data_sources) override;

  private:

    struct Camera
    {
      cv::Mat K;  // 3x3 intrinsics
      cv::Mat R;  // 3x3 rotation (world->camera)
      cv::Mat t;  // 3x1 translation (world->camera)
    };

    Camera refIntrinsics;
    Camera ocamIntrinsics;

    static cv::Mat PlaneHomography(const Camera& ref, const Camera& src, const cv::Vec3d& n, double d);
    void planeSweepDepth(const cv::Mat& refImg, const Camera& refCam, const std::vector<cv::Mat>& srcImgs, const std::vector<Camera>& srcCams, cv::Mat& depthMap, double zMin, double zMax, int nPlanes);
};
