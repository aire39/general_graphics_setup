#pragma once

#include <memory>

class FImage;

namespace ggs::image::math {

  std::shared_ptr<FImage> Multiply(const FImage* src_0, const FImage* src_1);

}