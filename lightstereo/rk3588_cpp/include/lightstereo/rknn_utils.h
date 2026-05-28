#pragma once
#include <rknn_api.h>
#include <string>
#include <vector>

namespace lightstereo {

std::string GetRknnErrorString(int code);
void PrintRknnTensorAttr(const rknn_tensor_attr& attr, const char* name);
std::vector<float> DequantizeInt8ToFloat(const int8_t* data, size_t size, int32_t zp, float scale);

}  // namespace lightstereo
