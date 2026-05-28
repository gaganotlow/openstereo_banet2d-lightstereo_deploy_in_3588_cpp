#include "lightstereo/lightstereo_infer.h"
#include "lightstereo/image_utils.h"
#include <cstdio>
#include <cstdlib>
#include <string>

int main(int argc, char** argv) {
  std::string model = "./models/lightstereo_m_orin_480x640.rknn";
  std::string left = "./samples/left.png";
  std::string right = "./samples/right.png";
  std::string output = "./output/demo_vis.png";
  int warmup = 1;
  int repeat = 1;
  int width = 640;
  int height = 480;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--model" && i + 1 < argc) model = argv[++i];
    else if (arg == "--left" && i + 1 < argc) left = argv[++i];
    else if (arg == "--right" && i + 1 < argc) right = argv[++i];
    else if (arg == "--output" && i + 1 < argc) output = argv[++i];
    else if (arg == "--warmup" && i + 1 < argc) warmup = std::atoi(argv[++i]);
    else if (arg == "--repeat" && i + 1 < argc) repeat = std::atoi(argv[++i]);
    else if (arg == "--width" && i + 1 < argc) width = std::atoi(argv[++i]);
    else if (arg == "--height" && i + 1 < argc) height = std::atoi(argv[++i]);
  }

  try {
    lightstereo::InferConfig config;
    config.model_path = model;
    config.width = width;
    config.height = height;
    config.num_threads = 3;
    config.core_mask = 0;

    lightstereo::LightStereoInfer infer(config);

    cv::Mat left_img = lightstereo::LoadImage(left, width, height, true);
    cv::Mat right_img = lightstereo::LoadImage(right, width, height, true);

    if (left_img.empty() || right_img.empty()) {
      std::fprintf(stderr, "Failed to load images\n");
      return 1;
    }

    for (int i = 0; i < warmup; ++i) {
      infer.Infer(left_img.data, right_img.data);
    }

    float total_time = 0.0f;
    lightstereo::InferResult result;
    for (int i = 0; i < repeat; ++i) {
      result = infer.Infer(left_img.data, right_img.data);
      total_time += result.inference_time_ms;
    }

    std::printf("Inference time: %.2f ms (avg over %d runs)\n", total_time / repeat, repeat);
    std::printf("FPS: %.2f\n", 1000.0f / (total_time / repeat));

    lightstereo::SaveDisparityImage(result.disparity.data(), width, height, output);
    std::printf("Saved: %s\n", output.c_str());

    return 0;
  } catch (const std::exception& e) {
    std::fprintf(stderr, "Error: %s\n", e.what());
    return 1;
  }
}
