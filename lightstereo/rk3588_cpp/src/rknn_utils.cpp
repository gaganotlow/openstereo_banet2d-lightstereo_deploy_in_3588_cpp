#include "lightstereo/rknn_utils.h"
#include <cstdio>

namespace lightstereo {

std::string GetRknnErrorString(int code) {
  switch (code) {
    case RKNN_SUCC: return "RKNN_SUCC";
    case RKNN_ERR_FAIL: return "RKNN_ERR_FAIL";
    case RKNN_ERR_TIMEOUT: return "RKNN_ERR_TIMEOUT";
    case RKNN_ERR_DEVICE_UNAVAILABLE: return "RKNN_ERR_DEVICE_UNAVAILABLE";
    case RKNN_ERR_MALLOC_FAIL: return "RKNN_ERR_MALLOC_FAIL";
    case RKNN_ERR_PARAM_INVALID: return "RKNN_ERR_PARAM_INVALID";
    case RKNN_ERR_MODEL_INVALID: return "RKNN_ERR_MODEL_INVALID";
    case RKNN_ERR_CTX_INVALID: return "RKNN_ERR_CTX_INVALID";
    case RKNN_ERR_INPUT_INVALID: return "RKNN_ERR_INPUT_INVALID";
    case RKNN_ERR_OUTPUT_INVALID: return "RKNN_ERR_OUTPUT_INVALID";
    default: return "UNKNOWN_ERROR";
  }
}

void PrintRknnTensorAttr(const rknn_tensor_attr& attr, const char* name) {
  std::printf("%s: dims=[%d,%d,%d,%d] type=%d qnt_type=%d zp=%d scale=%f\n",
              name, attr.dims[0], attr.dims[1], attr.dims[2], attr.dims[3],
              attr.type, attr.qnt_type, attr.zp, attr.scale);
}

std::vector<float> DequantizeInt8ToFloat(const int8_t* data, size_t size, int32_t zp, float scale) {
  std::vector<float> result(size);
  for (size_t i = 0; i < size; ++i) {
    result[i] = (data[i] - zp) * scale;
  }
  return result;
}

}  // namespace lightstereo
