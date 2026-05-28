#pragma once

#include "banet2d/types.h"
#include "rknn_api.h"

#include <opencv2/core.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace banet2d {

class Banet2dInfer {
 public:
  explicit Banet2dInfer(InferConfig config);
  ~Banet2dInfer();

  Banet2dInfer(const Banet2dInfer&) = delete;
  Banet2dInfer& operator=(const Banet2dInfer&) = delete;

  bool Init();
  DisparityResult Run(const cv::Mat& left_rgb, const cv::Mat& right_rgb);
  DisparityResult Run(const std::vector<uint8_t>& left_rgb, const std::vector<uint8_t>& right_rgb);

 private:
  bool LoadModel();
  bool QueryModelInfo();
  bool ValidateModelIo() const;
  ImageSize ParseOutputSize() const;

  InferConfig config_;
  rknn_context ctx_ = 0;
  std::vector<uint8_t> model_data_;
  rknn_input_output_num io_num_{};
  std::vector<rknn_tensor_attr> input_attrs_;
  std::vector<rknn_tensor_attr> output_attrs_;
  bool initialized_ = false;
};

}  // namespace banet2d
