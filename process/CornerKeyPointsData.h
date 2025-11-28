#pragma once

#include <vector>
#include "GenericData.h"
#include "opencv2/opencv.hpp"

struct CornerKeyPointsData : GenericData
{
  std::vector<cv::KeyPoint> keypoints;
};