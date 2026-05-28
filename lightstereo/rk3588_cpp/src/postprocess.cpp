#include "lightstereo/postprocess.h"
#include <algorithm>
#include <cmath>

namespace lightstereo {

std::vector<float> PostprocessDisparity(const int8_t* output_data, int width, int height,
                                        int32_t zp, float scale) {
  std::vector<float> disparity(width * height);
  for (int i = 0; i < width * height; ++i) {
    disparity[i] = (output_data[i] - zp) * scale;
  }
  return disparity;
}

}  // namespace lightstereo
