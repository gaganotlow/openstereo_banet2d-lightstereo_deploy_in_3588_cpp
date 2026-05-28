#pragma once

#include "banet2d/types.h"

#include <opencv2/core.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace banet2d {

cv::Mat LoadImageBgr(const std::string& path);
cv::Mat PrepareRgbImage(const std::string& path, const ImageSize& size, bool allow_resize);
std::vector<uint8_t> MatToRgbNhwcBuffer(const cv::Mat& rgb);

}  // namespace banet2d
