#pragma once

/**
 * Linux specific checks
*/

#if ENABLE_LINUX_PLATFORM_CHECKS && (defined(__linux__) || defined(__unix__))

#include <string>
#include <fstream>
#include <filesystem>
#include <linux/usbdevice_fs.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace gss::camera::checks {

  inline std::string get_device_path(int32_t fd)
  {
    std::string device_path = "";
    char path[1024];
    snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);
    char buf[1024];
    ssize_t len = readlink(path, buf, sizeof(buf) - 1);
    if (len != -1)
    {
        buf[len] = '\0';
        device_path = std::string(buf);  // e.g., "/dev/video0"
    }

    return device_path;
  }

  inline std::pair<std::string, std::string> get_bus_and_dev_from_video_device(const std::string& video_dev_path)
  {
      namespace fs = std::filesystem;

      try {
          // Resolve symlink for the video device in sysfs
          //fs::path videoSysPath = fs::read_symlink("/sys/class/video4linux/" + video_dev_path);
          fs::path videoSysPath = "/sys/class/video4linux/" + video_dev_path;
          fs::path fullSysPath = "/sys/class/video4linux/" + video_dev_path;
          fullSysPath = fs::canonical(fullSysPath);

          // Walk up directories to find USB device directory (matches USB pattern like "2-1")
          fs::path parent = fullSysPath.parent_path();
          while (!parent.empty()) {
              if (fs::exists(parent / "busnum") && fs::exists(parent / "devnum")) {
                  // Found USB device directory
                  std::ifstream busnumFile(parent / "busnum");
                  std::ifstream devnumFile(parent / "devnum");
                  std::string busnum, devnum;
                  std::getline(busnumFile, busnum);
                  std::getline(devnumFile, devnum);
                  return {busnum, devnum};
              }
              parent = parent.parent_path();
          }
      }
      catch (const std::exception& ex) {
          spdlog::error("Error resolving sysfs path: {}" ,ex.what());
      }
      return {"", ""};
  }

  bool inline reset_camera_usb(std::string busnum, std::string devnum)
  {
    busnum = (busnum.size() > 2) ? (std::string("0") + busnum) : (std::string("00") + busnum);
    devnum = (devnum.size() > 2) ? (std::string("00") + devnum) : (std::string("0") + devnum);

    bool success = true;
    const std::string dev_path = "/dev/bus/usb/" + busnum + "/" + devnum;
    int usb_fd = open(dev_path.c_str(), O_WRONLY);
    if (usb_fd < 0)
    {
        spdlog::error("Failed to open USB device node {}: {}", dev_path, strerror(errno));
        success = false;
    }

    if (success)
    {
      int rc = ioctl(usb_fd, USBDEVFS_RESET, 0);
      if (rc < 0)
      {
          spdlog::error("USBDEVFS_RESET failed on {}: {}", dev_path, strerror(errno));
          success = false;
      }
      else
      {
          spdlog::info("USB device {} reset successfully", dev_path);
      }
    }

    if (success)
    {
      close(usb_fd);
    }

    return success;
  }

  inline bool reset_camera_usb_from_path(const std::string& device_path)
  {
    auto bus_and_dev = get_bus_and_dev_from_video_device(device_path);
    return reset_camera_usb(bus_and_dev.first, bus_and_dev.second);
  }
}
#endif

/**
 * Window specific checks
*/

#if ENABLE_WINDOWS_PLATFORM_CHECKS
namespace gss::camera::checks {

}
#endif
