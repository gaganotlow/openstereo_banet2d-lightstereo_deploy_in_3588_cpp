#pragma once

#include <string>
#include <vector>

namespace banet2d {

struct ImageSize {
  int width = 640;
  int height = 480;
};

struct InferConfig {
  std::string model_path = "./models/checkpoint_epoch_195_int8.rknn";
  ImageSize input_size;
  bool dump_tensor_info = true;
};

struct DisparityResult {
  int width = 0;
  int height = 0;
  std::vector<float> disparity;
  double inference_ms = 0.0;
};

struct DisparityStats {
  float min = 0.0f;
  float max = 0.0f;
  float mean = 0.0f;
};

}  // namespace banet2d
