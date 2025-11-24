#include "SceneDepthBlock.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <SDL3/SDL.h>
#include "graphics/images/FImage.h"
#include "graphics/filters/Filters.h"
#include "support/logging.h"

namespace {
  constexpr float lpf_smooth_factor = 0.1f;
}

SceneDepthBlock::SceneDepthBlock(const std::string &thread_name, const std::string &thread_description)
  : SceneDepthBlock(thread_name, thread_description, {})
{
  name = thread_name;
  description = thread_description;
}

SceneDepthBlock::SceneDepthBlock(const std::string &thread_name, const std::string &thread_description, const std::vector<std::shared_ptr<ImageProcessBlock>> &others)
{
  name = thread_name;
  description = thread_description;
  connections = others;
  dataFlow = DataFlow::F_INOUT;

  refIntrinsics.K = (cv::Mat_<double>(3, 3) <<
    554.0, 0.0, 320.0,
    0.0, 554.0, 240.0,
    0.0, 0.0, 1.0);

  double angle_deg = 0.2;
  double angle_rad = angle_deg * CV_PI / 180.0;
  cv::Mat rvec = (cv::Mat_<double>(3,1) << 0.0, angle_rad, 0.0);

  cv::Mat R;
  cv::Rodrigues(rvec, R);

  refIntrinsics.R = (cv::Mat_<double>(3, 3) <<
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0);

  refIntrinsics.t = (cv::Mat_<double>(3, 1) <<
    0.0, 0.0, 0.0);

  ocamIntrinsics.K = (cv::Mat_<double>(3, 3) <<
    554.0, 0.0, 320.0,
    0.0, 554.0, 240.0,
    0.0, 0.0, 1.0);

  ocamIntrinsics.R = (cv::Mat_<double>(3, 3) <<
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0);

  ocamIntrinsics.t = (cv::Mat_<double>(3, 1) <<
    0.0635, 0.0, 0.0);
}

void SceneDepthBlock::SetCameraRefPosition(const std::array<float, 3> position)
{
  refIntrinsics.t = (cv::Mat_<double>(3, 1) << position[0], position[1], position[2]);
}

void SceneDepthBlock::SetCameraRefFocal(const std::array<float, 2> focal)
{
  refIntrinsics.K = (cv::Mat_<double>(3, 3) <<
  focal[0], 0.0, 320.0,
  0.0, focal[1], 240.0,
  0.0, 0.0, 1.0);
}

void SceneDepthBlock::SetCameraPosition(const std::array<float, 3> position, int32_t index)
{
  if (index == 0)
  {
    ocamIntrinsics.t = (cv::Mat_<double>(3, 1) << position[0], position[1], position[2]);
  }
}

void SceneDepthBlock::SetCameraFocal(const std::array<float, 2> focal, int32_t index)
{
  if (index == 0)
  {
    ocamIntrinsics.K = (cv::Mat_<double>(3, 3) <<
    focal[0], 0.0, 320.0,
    0.0, focal[1], 240.0,
    0.0, 0.0, 1.0);
  }
}

std::vector<std::shared_ptr<FImage>> SceneDepthBlock::Process(std::vector<std::shared_ptr<FImage>> image_sources, [[maybe_unused]] DataContainer &data_sources)
{
  std::shared_ptr<FImage> ref_source = nullptr;
  std::shared_ptr<FImage> cam_source = nullptr;
  if (!image_sources.empty() && image_sources.size() > 1)
  {
    ref_source = image_sources[0];
    cam_source = image_sources[1];
  }
  else
  {
    return {nullptr};
  }

  const auto start_time_process = std::chrono::high_resolution_clock::now();

  auto ref_source_rgb = std::make_shared<FImage>("ref_rgb", ref_source->GetWidth(), ref_source->GetHeight(), SDL_PIXELFORMAT_RGB24, true);
  ref_source_rgb->ProcessFilterFromImage(ref_source.get(), 0, 0, filter::functions::cpu::parallel_vectorize::uyvy_to_rgb_conversion);

  auto cam_source_rgb = std::make_shared<FImage>("ref_rgb", cam_source->GetWidth(), cam_source->GetHeight(), SDL_PIXELFORMAT_RGB24, true);
  cam_source_rgb->ProcessFilterFromImage(cam_source.get(), 0, 0, filter::functions::cpu::parallel_vectorize::yuy2_to_rgb_conversion);

  cv::Mat ref_gray;
  const cv::Mat ref_image(ref_source_rgb->GetHeight(), ref_source_rgb->GetWidth(), CV_8UC3, ref_source_rgb->GetImage(0)->pixels);
  cv::cvtColor(ref_image, ref_gray, cv::COLOR_RGB2GRAY);
  ref_gray.convertTo(ref_gray, CV_32F, 1.0/255.0);

  cv::Mat cam_gray;
  const cv::Mat cam_image(cam_source_rgb->GetHeight(), cam_source_rgb->GetWidth(), CV_8UC3, cam_source_rgb->GetImage(0)->pixels);
  cv::cvtColor(cam_image, cam_gray, cv::COLOR_RGB2GRAY);
  cam_gray.convertTo(cam_gray, CV_32F, 1.0/255.0);

  cv::Mat depth_map;
  planeSweepDepth(ref_gray, refIntrinsics, {cam_gray}, {ocamIntrinsics}, depth_map, 0.2, 5.0, 128);

  cv::Mat gray_rgb;
  cv::cvtColor(depth_map, gray_rgb, cv::COLOR_GRAY2RGB);
  gray_rgb.convertTo(gray_rgb, CV_8UC3, 255.0);

  auto depth_result = std::make_shared<FImage>("gray", gray_rgb.cols, gray_rgb.rows, SDL_PixelFormat::SDL_PIXELFORMAT_RGB24, true);
  std::memcpy(depth_result->GetImage(0)->pixels, gray_rgb.data, depth_result->GetHeight() * depth_result->GetWidth() * 3);

  // timing information

  const auto end_time_process = std::chrono::high_resolution_clock::now();
  const int64_t tick_count = std::chrono::duration_cast<std::chrono::microseconds>(end_time_process - start_time_process).count();
  timeToComplete = static_cast<float>(tick_count) / 1000.0f;
  timeFilterToComplete = (lpf_smooth_factor * timeToComplete) + (1.0f - lpf_smooth_factor) * timeFilterToComplete;

  return {depth_result};
}

  cv::Mat SceneDepthBlock::PlaneHomography(const Camera& ref, const Camera& src, const cv::Vec3d& n, double d)
  {
    // All matrices are CV_64F
    cv::Mat Kr = ref.K, Ks = src.K;
    cv::Mat Rr = ref.R, Rs = src.R;
    cv::Mat tr = ref.t, ts = src.t;

    // Compute (R - t*n^T/d)
    cv::Mat Rr_ = Rr - (tr * cv::Mat(n).t()) / d;
    cv::Mat Rs_ = Rs - (ts * cv::Mat(n).t()) / d;

    cv::Mat H = Kr * Rr_ * Rs_.inv() * Ks.inv();
    return H;
  }

  void SceneDepthBlock::planeSweepDepth(
      const cv::Mat& refImg,
      const Camera& refCam,
      const std::vector<cv::Mat>& srcImgs,
      const std::vector<Camera>& srcCams,
      cv::Mat& depthMap,
      double zMin, double zMax, int nPlanes)
  {
    const cv::Vec3d n(0, 0, 1);  // plane normal
    cv::Mat bestScore(refImg.size(), CV_32F, cv::Scalar(-1e9));
    depthMap = cv::Mat(refImg.size(), CV_32F, cv::Scalar(zMin));

    for (int p = 0; p < nPlanes; ++p)
    {
      double z = zMin + (zMax - zMin) * p / (nPlanes - 1);
      double d = -z;

      cv::Mat accum = cv::Mat::zeros(refImg.size(), CV_32F);

      // Warp each source image into reference
      for (size_t i = 0; i < srcImgs.size(); ++i)
      {
        cv::Mat H = PlaneHomography(refCam, srcCams[i], n, d);
        cv::Mat warped;
        cv::warpPerspective(srcImgs[i], warped, H, refImg.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, 0);

        // Compute photo-consistency (negative absolute difference)
        cv::Mat diff;
        cv::absdiff(refImg, warped, diff);
        diff.convertTo(diff, CV_32F);
        accum += -diff;  // higher = more similar
      }

      // Update the best depth for each pixel
      for (int y = 0; y < refImg.rows; ++y) {
        const float* accPtr = accum.ptr<float>(y);
        float* bestPtr = bestScore.ptr<float>(y);
        float* depthPtr = depthMap.ptr<float>(y);

        for (int x = 0; x < refImg.cols; ++x) {
          if (accPtr[x] > bestPtr[x]) {
            bestPtr[x] = accPtr[x];
            depthPtr[x] = static_cast<float>(z);
          }
        }
      }

      logging::info("Plane {}/{} done (z={})", p+1, nPlanes, z);
    }
  }
