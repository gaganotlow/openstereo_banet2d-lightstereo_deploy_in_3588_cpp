#pragma once
#include <opencv2/opencv.hpp>
#include <string>

namespace lightstereo {

cv::Mat LoadImage(const std::string& path, int target_width, int target_height, bool resize);
void SaveDisparityImage(const float* disp, int width, int height, const std::string& path);

}  // namespace lightstereo
