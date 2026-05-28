#include "banet2d/image_utils.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace banet2d {

cv::Mat LoadImageBgr(const std::string& path) {
  cv::Mat bgr = cv::imread(path, cv::IMREAD_COLOR);
  if (bgr.empty()) {
    throw std::runtime_error("failed to load image: " + path);
  }
  return bgr;
}

cv::Mat PrepareRgbImage(const std::string& path, const ImageSize& size, bool allow_resize) {
  cv::Mat bgr = LoadImageBgr(path);
  if (bgr.cols != size.width || bgr.rows != size.height) {
    if (!allow_resize) {
      char msg[256];
      std::snprintf(msg, sizeof(msg), "image size mismatch for %s: got %dx%d, expected %dx%d",
                    path.c_str(), bgr.cols, bgr.rows, size.width, size.height);
      throw std::runtime_error(msg);
    }
    cv::resize(bgr, bgr, cv::Size(size.width, size.height), 0, 0, cv::INTER_LINEAR);
  }

  cv::Mat rgb;
  cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
  return rgb;
}

std::vector<uint8_t> MatToRgbNhwcBuffer(const cv::Mat& rgb) {
  if (rgb.empty() || rgb.type() != CV_8UC3) {
    throw std::runtime_error("expected non-empty CV_8UC3 RGB image");
  }
  cv::Mat contiguous = rgb.isContinuous() ? rgb : rgb.clone();
  const size_t bytes = static_cast<size_t>(contiguous.rows) * contiguous.cols * contiguous.channels();
  std::vector<uint8_t> buffer(bytes);
  std::memcpy(buffer.data(), contiguous.data, bytes);
  return buffer;
}

}  // namespace banet2d
