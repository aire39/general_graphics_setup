#pragma once

#include <cstdint>
#include <array>

class VaryingBitsMenu
{
  public:
    VaryingBitsMenu();

    void RenderMenu();
    [[nodiscard]] int32_t BitScale() const;
    [[nodiscard]] bool ShiftBitsForContrast() const;
    bool ProcessBegin();

    [[nodiscard]] bool UseColorChannels() const;
    [[nodiscard]] const std::array<bool, 8> & ShowBitPlanes() const;
    [[nodiscard]] uint8_t BitPlaneMask() const;
    [[nodiscard]] bool ShowHidePlaneMode() const;

    bool ForceBitScaleUpdate();

private:
    bool processBegin = false;
    bool shiftBitsContrast = true;
    int32_t setBit = 8;
    bool useColor = false;
    bool showHidePlaneMode = false;
    bool showBitPlaneMode = true;
    bool forceBitScaleUpdate = false;
    std::array<bool, 8> showBitPlaneOption {};
};
