#pragma once
#include <cstdint>
#include <vector>

namespace lightstereo {

std::vector<float> PostprocessDisparity(const int8_t* output_data, int width, int height,
                                        int32_t zp, float scale);

}  // namespace lightstereo
