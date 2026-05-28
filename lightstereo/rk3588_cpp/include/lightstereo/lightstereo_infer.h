#pragma once
#include "lightstereo/types.h"
#include <rknn_api.h>
#include <memory>

namespace lightstereo {

class LightStereoInfer {
public:
  explicit LightStereoInfer(const InferConfig& config);
  ~LightStereoInfer();

  InferResult Infer(const unsigned char* left_rgb, const unsigned char* right_rgb);

private:
  InferConfig config_;
  rknn_context ctx_ = 0;
  rknn_input_output_num io_num_;
  rknn_tensor_attr* input_attrs_ = nullptr;
  rknn_tensor_attr* output_attrs_ = nullptr;
};

}  // namespace lightstereo
