#pragma once

#include "rknn_api.h"

#include <string>

namespace banet2d {

std::string TensorFormatToString(rknn_tensor_format fmt);
std::string TensorTypeToString(rknn_tensor_type type);
std::string QuantTypeToString(rknn_tensor_qnt_type type);

bool CheckRknnRet(int ret, const std::string& where);
void DumpTensorAttr(const std::string& tag, const rknn_tensor_attr& attr);
void PrintSdkVersion(rknn_context ctx);

}  // namespace banet2d
