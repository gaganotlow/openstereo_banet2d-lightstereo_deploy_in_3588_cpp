#include "banet2d/rknn_utils.h"

#include <cstdio>

namespace banet2d {

std::string TensorFormatToString(rknn_tensor_format fmt) {
  switch (fmt) {
    case RKNN_TENSOR_NCHW: return "NCHW";
    case RKNN_TENSOR_NHWC: return "NHWC";
    case RKNN_TENSOR_NC1HWC2: return "NC1HWC2";
    case RKNN_TENSOR_UNDEFINED: return "UNDEFINED";
    default: return "UNKNOWN";
  }
}

std::string TensorTypeToString(rknn_tensor_type type) {
  switch (type) {
    case RKNN_TENSOR_FLOAT32: return "FLOAT32";
    case RKNN_TENSOR_FLOAT16: return "FLOAT16";
    case RKNN_TENSOR_INT8: return "INT8";
    case RKNN_TENSOR_UINT8: return "UINT8";
    case RKNN_TENSOR_INT16: return "INT16";
    case RKNN_TENSOR_UINT16: return "UINT16";
    case RKNN_TENSOR_INT32: return "INT32";
    case RKNN_TENSOR_UINT32: return "UINT32";
    default: return "UNKNOWN";
  }
}

std::string QuantTypeToString(rknn_tensor_qnt_type type) {
  switch (type) {
    case RKNN_TENSOR_QNT_NONE: return "NONE";
    case RKNN_TENSOR_QNT_DFP: return "DFP";
    case RKNN_TENSOR_QNT_AFFINE_ASYMMETRIC: return "AFFINE_ASYMMETRIC";
    default: return "UNKNOWN";
  }
}

bool CheckRknnRet(int ret, const std::string& where) {
  if (ret == RKNN_SUCC) return true;
  std::fprintf(stderr, "%s failed: %d\n", where.c_str(), ret);
  return false;
}

void DumpTensorAttr(const std::string& tag, const rknn_tensor_attr& attr) {
  std::printf("%s index=%u name=%s n_dims=%u dims=[%u,%u,%u,%u] n_elems=%u size=%u fmt=%s type=%s qnt=%s zp=%d scale=%f\n",
              tag.c_str(), attr.index, attr.name, attr.n_dims,
              attr.dims[0], attr.dims[1], attr.dims[2], attr.dims[3],
              attr.n_elems, attr.size, TensorFormatToString(attr.fmt).c_str(),
              TensorTypeToString(attr.type).c_str(), QuantTypeToString(attr.qnt_type).c_str(),
              attr.zp, attr.scale);
}

void PrintSdkVersion(rknn_context ctx) {
  rknn_sdk_version sdk_ver;
  int ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &sdk_ver, sizeof(sdk_ver));
  if (ret == RKNN_SUCC) {
    std::printf("rknn api: %s, driver: %s\n", sdk_ver.api_version, sdk_ver.drv_version);
  }
}

}  // namespace banet2d
