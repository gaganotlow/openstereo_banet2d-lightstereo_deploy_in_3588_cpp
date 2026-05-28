#include "banet2d/banet2d_infer.h"

#include "banet2d/image_utils.h"
#include "banet2d/rknn_utils.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace banet2d {

Banet2dInfer::Banet2dInfer(InferConfig config) : config_(std::move(config)) {}

Banet2dInfer::~Banet2dInfer() {
  if (ctx_ != 0) {
    rknn_destroy(ctx_);
    ctx_ = 0;
  }
}

bool Banet2dInfer::LoadModel() {
  std::ifstream file(config_.model_path, std::ios::binary | std::ios::ate);
  if (!file) {
    std::fprintf(stderr, "failed to open model: %s\n", config_.model_path.c_str());
    return false;
  }
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  model_data_.resize(static_cast<size_t>(size));
  if (!file.read(reinterpret_cast<char*>(model_data_.data()), size)) {
    std::fprintf(stderr, "failed to read model: %s\n", config_.model_path.c_str());
    return false;
  }
  return true;
}

bool Banet2dInfer::Init() {
  if (initialized_) return true;
  if (!LoadModel()) return false;

  int ret = rknn_init(&ctx_, model_data_.data(), model_data_.size(), 0, nullptr);
  if (!CheckRknnRet(ret, "rknn_init")) return false;

  PrintSdkVersion(ctx_);
  if (!QueryModelInfo()) return false;
  if (!ValidateModelIo()) return false;
  initialized_ = true;
  return true;
}

bool Banet2dInfer::QueryModelInfo() {
  int ret = rknn_query(ctx_, RKNN_QUERY_IN_OUT_NUM, &io_num_, sizeof(io_num_));
  if (!CheckRknnRet(ret, "RKNN_QUERY_IN_OUT_NUM")) return false;

  std::printf("model input num: %u, output num: %u\n", io_num_.n_input, io_num_.n_output);

  input_attrs_.resize(io_num_.n_input);
  for (uint32_t i = 0; i < io_num_.n_input; ++i) {
    std::memset(&input_attrs_[i], 0, sizeof(rknn_tensor_attr));
    input_attrs_[i].index = i;
    ret = rknn_query(ctx_, RKNN_QUERY_INPUT_ATTR, &input_attrs_[i], sizeof(rknn_tensor_attr));
    if (!CheckRknnRet(ret, "RKNN_QUERY_INPUT_ATTR")) return false;
    if (config_.dump_tensor_info) DumpTensorAttr("input", input_attrs_[i]);
  }

  output_attrs_.resize(io_num_.n_output);
  for (uint32_t i = 0; i < io_num_.n_output; ++i) {
    std::memset(&output_attrs_[i], 0, sizeof(rknn_tensor_attr));
    output_attrs_[i].index = i;
    ret = rknn_query(ctx_, RKNN_QUERY_OUTPUT_ATTR, &output_attrs_[i], sizeof(rknn_tensor_attr));
    if (!CheckRknnRet(ret, "RKNN_QUERY_OUTPUT_ATTR")) return false;
    if (config_.dump_tensor_info) DumpTensorAttr("output", output_attrs_[i]);
  }
  return true;
}

bool Banet2dInfer::ValidateModelIo() const {
  if (io_num_.n_input != 2 || io_num_.n_output != 1) {
    std::fprintf(stderr, "expected 2 inputs and 1 output, got %u inputs and %u outputs\n",
                 io_num_.n_input, io_num_.n_output);
    return false;
  }
  return true;
}

ImageSize Banet2dInfer::ParseOutputSize() const {
  ImageSize size;
  if (output_attrs_.empty()) return size;
  const auto& attr = output_attrs_[0];
  if (attr.n_dims == 4) {
    if (attr.fmt == RKNN_TENSOR_NCHW) {
      size.height = static_cast<int>(attr.dims[2]);
      size.width = static_cast<int>(attr.dims[3]);
    } else {
      size.height = static_cast<int>(attr.dims[1]);
      size.width = static_cast<int>(attr.dims[2]);
    }
  }
  return size;
}

DisparityResult Banet2dInfer::Run(const cv::Mat& left_rgb, const cv::Mat& right_rgb) {
  return Run(MatToRgbNhwcBuffer(left_rgb), MatToRgbNhwcBuffer(right_rgb));
}

DisparityResult Banet2dInfer::Run(const std::vector<uint8_t>& left_rgb, const std::vector<uint8_t>& right_rgb) {
  if (!initialized_ && !Init()) {
    throw std::runtime_error("failed to initialize RKNN model");
  }

  const size_t input_bytes = static_cast<size_t>(config_.input_size.width) * config_.input_size.height * 3;
  if (left_rgb.size() != input_bytes || right_rgb.size() != input_bytes) {
    throw std::runtime_error("input buffer size mismatch");
  }

  rknn_input inputs[2];
  std::memset(inputs, 0, sizeof(inputs));
  inputs[0].index = 0;
  inputs[0].type = RKNN_TENSOR_UINT8;
  inputs[0].fmt = RKNN_TENSOR_NHWC;
  inputs[0].size = left_rgb.size();
  inputs[0].buf = const_cast<uint8_t*>(left_rgb.data());
  inputs[0].pass_through = 0;
  inputs[1].index = 1;
  inputs[1].type = RKNN_TENSOR_UINT8;
  inputs[1].fmt = RKNN_TENSOR_NHWC;
  inputs[1].size = right_rgb.size();
  inputs[1].buf = const_cast<uint8_t*>(right_rgb.data());
  inputs[1].pass_through = 0;

  int ret = rknn_inputs_set(ctx_, 2, inputs);
  if (!CheckRknnRet(ret, "rknn_inputs_set")) {
    throw std::runtime_error("rknn_inputs_set failed");
  }

  auto t0 = std::chrono::steady_clock::now();
  ret = rknn_run(ctx_, nullptr);
  auto t1 = std::chrono::steady_clock::now();
  if (!CheckRknnRet(ret, "rknn_run")) {
    throw std::runtime_error("rknn_run failed");
  }

  rknn_output output;
  std::memset(&output, 0, sizeof(output));
  output.index = 0;
  output.want_float = 1;
  output.is_prealloc = 0;
  ret = rknn_outputs_get(ctx_, 1, &output, nullptr);
  if (!CheckRknnRet(ret, "rknn_outputs_get")) {
    throw std::runtime_error("rknn_outputs_get failed");
  }

  ImageSize out_size = ParseOutputSize();
  size_t elem_count = static_cast<size_t>(out_size.width) * out_size.height;
  if (output.size < elem_count * sizeof(float)) {
    rknn_outputs_release(ctx_, 1, &output);
    throw std::runtime_error("output size is smaller than expected float disparity size");
  }

  const float* ptr = reinterpret_cast<const float*>(output.buf);
  DisparityResult result;
  result.width = out_size.width;
  result.height = out_size.height;
  result.disparity.assign(ptr, ptr + elem_count);
  result.inference_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

  rknn_outputs_release(ctx_, 1, &output);
  return result;
}

}  // namespace banet2d
