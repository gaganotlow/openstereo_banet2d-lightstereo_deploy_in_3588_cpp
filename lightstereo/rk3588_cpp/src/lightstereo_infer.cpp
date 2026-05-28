#include "lightstereo/lightstereo_infer.h"
#include "lightstereo/rknn_utils.h"
#include "lightstereo/postprocess.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <chrono>

namespace lightstereo {

LightStereoInfer::LightStereoInfer(const InferConfig& config) : config_(config) {
  std::ifstream file(config.model_path, std::ios::binary | std::ios::ate);
  if (!file) {
    throw std::runtime_error("Failed to open model file: " + config.model_path);
  }
  size_t model_size = file.tellg();
  file.seekg(0);
  std::vector<char> model_data(model_size);
  file.read(model_data.data(), model_size);

  int ret = rknn_init(&ctx_, model_data.data(), model_size, 0, nullptr);
  if (ret != RKNN_SUCC) {
    throw std::runtime_error("rknn_init failed: " + GetRknnErrorString(ret));
  }

  ret = rknn_query(ctx_, RKNN_QUERY_IN_OUT_NUM, &io_num_, sizeof(io_num_));
  if (ret != RKNN_SUCC || io_num_.n_input != 2 || io_num_.n_output != 1) {
    rknn_destroy(ctx_);
    throw std::runtime_error("Invalid model I/O");
  }

  input_attrs_ = new rknn_tensor_attr[io_num_.n_input];
  output_attrs_ = new rknn_tensor_attr[io_num_.n_output];

  for (uint32_t i = 0; i < io_num_.n_input; ++i) {
    input_attrs_[i].index = i;
    rknn_query(ctx_, RKNN_QUERY_INPUT_ATTR, &input_attrs_[i], sizeof(rknn_tensor_attr));
  }
  for (uint32_t i = 0; i < io_num_.n_output; ++i) {
    output_attrs_[i].index = i;
    rknn_query(ctx_, RKNN_QUERY_OUTPUT_ATTR, &output_attrs_[i], sizeof(rknn_tensor_attr));
  }

  if (config.num_threads > 0) {
    rknn_set_core_mask(ctx_, static_cast<rknn_core_mask>(config.core_mask));
  }
}

LightStereoInfer::~LightStereoInfer() {
  if (input_attrs_) delete[] input_attrs_;
  if (output_attrs_) delete[] output_attrs_;
  if (ctx_) rknn_destroy(ctx_);
}

InferResult LightStereoInfer::Infer(const unsigned char* left_rgb, const unsigned char* right_rgb) {
  InferResult result;
  result.width = config_.width;
  result.height = config_.height;

  rknn_input inputs[2];
  std::memset(inputs, 0, sizeof(inputs));

  inputs[0].index = 0;
  inputs[0].type = RKNN_TENSOR_UINT8;
  inputs[0].fmt = RKNN_TENSOR_NHWC;
  inputs[0].size = config_.width * config_.height * 3;
  inputs[0].buf = const_cast<unsigned char*>(left_rgb);

  inputs[1].index = 1;
  inputs[1].type = RKNN_TENSOR_UINT8;
  inputs[1].fmt = RKNN_TENSOR_NHWC;
  inputs[1].size = config_.width * config_.height * 3;
  inputs[1].buf = const_cast<unsigned char*>(right_rgb);

  int ret = rknn_inputs_set(ctx_, 2, inputs);
  if (ret != RKNN_SUCC) {
    throw std::runtime_error("rknn_inputs_set failed");
  }

  auto start = std::chrono::high_resolution_clock::now();
  ret = rknn_run(ctx_, nullptr);
  auto end = std::chrono::high_resolution_clock::now();

  if (ret != RKNN_SUCC) {
    throw std::runtime_error("rknn_run failed");
  }

  result.inference_time_ms = std::chrono::duration<float, std::milli>(end - start).count();

  rknn_output outputs[1];
  std::memset(outputs, 0, sizeof(outputs));
  outputs[0].want_float = 0;
  outputs[0].is_prealloc = 0;

  ret = rknn_outputs_get(ctx_, 1, outputs, nullptr);
  if (ret != RKNN_SUCC) {
    throw std::runtime_error("rknn_outputs_get failed");
  }

  result.disparity = PostprocessDisparity(
      static_cast<const int8_t*>(outputs[0].buf),
      config_.width, config_.height,
      output_attrs_[0].zp, output_attrs_[0].scale);

  rknn_outputs_release(ctx_, 1, outputs);
  return result;
}

}  // namespace lightstereo
