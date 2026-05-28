#include "lightstereo/image_utils.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <cstdio>

namespace lightstereo {

cv::Mat LoadImage(const std::string& path, int target_width, int target_height, bool resize) {
  cv::Mat img = cv::imread(path, cv::IMREAD_COLOR);
  if (img.empty()) {
    std::fprintf(stderr, "Failed to load image: %s\n", path.c_str());
    return cv::Mat();
  }
  if (resize && (img.cols != target_width || img.rows != target_height)) {
    cv::resize(img, img, cv::Size(target_width, target_height));
  }
  return img;
}

void SaveDisparityImage(const float* disp, int width, int height, const std::string& path) {
  cv::Mat vis(height, width, CV_8UC1);
  float max_disp = 192.0f;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      float v = disp[y * width + x];
      v = std::max(0.0f, std::min(v, max_disp));
      vis.at<uint8_t>(y, x) = static_cast<uint8_t>(v / max_disp * 255.0f);
    }
  }
  cv::imwrite(path, vis);
}

}  // namespace lightstereo
