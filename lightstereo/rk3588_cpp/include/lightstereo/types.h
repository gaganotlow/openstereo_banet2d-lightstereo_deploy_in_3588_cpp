#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace lightstereo {

struct InferConfig {
  std::string model_path;
  int width = 640;
  int height = 480;
  int num_threads = 3;
  int core_mask = 0;
};

struct InferResult {
  std::vector<float> disparity;
  int width;
  int height;
  float inference_time_ms;
};

}  // namespace lightstereo
