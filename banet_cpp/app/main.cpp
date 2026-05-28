#include "banet2d/banet2d_infer.h"
#include "banet2d/image_utils.h"
#include "banet2d/postprocess.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace {

struct Args {
  std::string model = "./models/checkpoint_epoch_195_optimized_fp16.rknn";
  std::string left = "./samples/left.png";
  std::string right = "./samples/right.png";
  std::string output = "./output/demo";
  int warmup = 1;
  int repeat = 1;
  bool resize = false;
  bool dump_tensor = true;
};

void PrintUsage(const char* prog) {
  std::printf("Usage: %s [options]\n", prog);
  std::printf("Options:\n");
  std::printf("  --model PATH        RKNN model path (default: ./models/checkpoint_epoch_195_optimized_fp16.rknn)\n");
  std::printf("  --left PATH         left image path (default: ./samples/left.png)\n");
  std::printf("  --right PATH        right image path (default: ./samples/right.png)\n");
  std::printf("  --output PREFIX     output prefix (default: ./output/demo)\n");
  std::printf("  --warmup N          warmup runs (default: 1)\n");
  std::printf("  --repeat N          measured runs (default: 1)\n");
  std::printf("  --resize            resize images to 640x480 if needed\n");
  std::printf("  --no-dump-tensor    do not print tensor attrs\n");
  std::printf("  --help              show this message\n");
}

bool ParseArgs(int argc, char** argv, Args* args) {
  for (int i = 1; i < argc; ++i) {
    std::string key = argv[i];
    auto need_value = [&](const char* name) -> const char* {
      if (i + 1 >= argc) {
        std::fprintf(stderr, "%s requires a value\n", name);
        return nullptr;
      }
      return argv[++i];
    };

    if (key == "--help" || key == "-h") {
      PrintUsage(argv[0]);
      std::exit(0);
    } else if (key == "--model") {
      const char* v = need_value("--model");
      if (!v) return false;
      args->model = v;
    } else if (key == "--left") {
      const char* v = need_value("--left");
      if (!v) return false;
      args->left = v;
    } else if (key == "--right") {
      const char* v = need_value("--right");
      if (!v) return false;
      args->right = v;
    } else if (key == "--output") {
      const char* v = need_value("--output");
      if (!v) return false;
      args->output = v;
    } else if (key == "--warmup") {
      const char* v = need_value("--warmup");
      if (!v) return false;
      args->warmup = std::atoi(v);
    } else if (key == "--repeat") {
      const char* v = need_value("--repeat");
      if (!v) return false;
      args->repeat = std::atoi(v);
    } else if (key == "--resize") {
      args->resize = true;
    } else if (key == "--no-dump-tensor") {
      args->dump_tensor = false;
    } else {
      std::fprintf(stderr, "unknown option: %s\n", key.c_str());
      return false;
    }
  }
  if (args->warmup < 0) args->warmup = 0;
  if (args->repeat <= 0) args->repeat = 1;
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  Args args;
  if (!ParseArgs(argc, argv, &args)) {
    PrintUsage(argv[0]);
    return 1;
  }

  try {
    banet2d::InferConfig config;
    config.model_path = args.model;
    config.dump_tensor_info = args.dump_tensor;

    cv::Mat left_rgb = banet2d::PrepareRgbImage(args.left, config.input_size, args.resize);
    cv::Mat right_rgb = banet2d::PrepareRgbImage(args.right, config.input_size, args.resize);

    banet2d::Banet2dInfer infer(config);
    if (!infer.Init()) return 1;

    for (int i = 0; i < args.warmup; ++i) {
      (void)infer.Run(left_rgb, right_rgb);
    }

    banet2d::DisparityResult result;
    double total_ms = 0.0;
    for (int i = 0; i < args.repeat; ++i) {
      result = infer.Run(left_rgb, right_rgb);
      total_ms += result.inference_ms;
    }
    result.inference_ms = total_ms / args.repeat;
    std::printf("average rknn_run: %.3f ms over %d runs\n", result.inference_ms, args.repeat);

    if (!banet2d::SaveDisparityOutputs(args.output, result, args.model, args.left, args.right)) {
      std::fprintf(stderr, "failed to save one or more output files\n");
      return 1;
    }
  } catch (const std::exception& e) {
    std::fprintf(stderr, "error: %s\n", e.what());
    return 1;
  }

  return 0;
}
