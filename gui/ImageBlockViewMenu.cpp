#include "ImageBlockViewMenu.h"

#include <imgui.h>
#include "process/ImageProcessBlock.h"
#include "process/NonMaximumSuppressBlock.h"
#include "process/OpticalFlowBlock.h"

ImageBlockViewMenu::ImageBlockViewMenu(const std::vector<ImageProcessBlock*> &image_blocks)
  : imageBlocks(image_blocks)
{
  viewIndex = std::max(static_cast<int32_t>(imageBlocks.size() - 1), 0);
}

void ImageBlockViewMenu::RenderMenu()
{
  auto nms_block = dynamic_cast<NonMaximumSuppressBlock*>(imageBlocks[static_cast<uint32_t>(viewIndex)]);
  auto optical_block = dynamic_cast<OpticalFlowBlock*>(imageBlocks[static_cast<uint32_t>(viewIndex)]);

  if (!imageBlocks.empty())
  {
    ImGui::Begin("ImageBlock Controller", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushID(1);
    ImGui::BeginGroup();
    ImGui::SliderInt("View Index", &viewIndex, 0, static_cast<int32_t>(imageBlocks.size() - 1), "%d");
    ImGui::Text("process time: %.2fms (%.2fms)", imageBlocks[static_cast<uint32_t>(viewIndex)]->TimeToComplete(), imageBlocks[static_cast<uint32_t>(viewIndex)]->TimeToCompleteFilter());
    if (nms_block) ImGui::Text("number of keypoints: %d", nms_block->GetNumberOfKeypoints());
    if (optical_block) ImGui::Text("number of keypoints: %d", optical_block->GetNumberOfGoodTracks());
    ImGui::EndGroup();
    ImGui::PopID();
    ImGui::End();
  }
}

std::shared_ptr<FImage> ImageBlockViewMenu::GetImage() const
{
  std::shared_ptr<FImage> image = nullptr;

  if (!imageBlocks.empty())
  {
    image = imageBlocks[static_cast<size_t>(viewIndex)]->GetLastImage();

    for (size_t i = 0; i < imageBlocks.size(); ++i)
    {
      if (i != static_cast<size_t>(viewIndex))
      {
        imageBlocks[i]->GetLastImage(); // drop frame from queue
      }
    }
  }

  return image;
}
