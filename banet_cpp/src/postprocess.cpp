#include "banet2d/postprocess.h"

#include <opencv2/imgcodecs.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>

namespace banet2d {

DisparityStats ComputeStats(const DisparityResult& result) {
  DisparityStats stats;
  if (result.disparity.empty()) return stats;

  stats.min = std::numeric_limits<float>::infinity();
  stats.max = -std::numeric_limits<float>::infinity();
  double sum = 0.0;
  int count = 0;
  for (float v : result.disparity) {
    if (!std::isfinite(v)) continue;
    stats.min = std::min(stats.min, v);
    stats.max = std::max(stats.max, v);
    sum += v;
    ++count;
  }
  if (count == 0) {
    stats.min = 0.0f;
    stats.max = 0.0f;
    stats.mean = 0.0f;
  } else {
    stats.mean = static_cast<float>(sum / count);
  }
  return stats;
}

bool SaveDisparityBin(const std::string& path, const DisparityResult& result) {
  std::ofstream out(path, std::ios::binary);
  if (!out) return false;
  out.write(reinterpret_cast<const char*>(result.disparity.data()),
            static_cast<std::streamsize>(result.disparity.size() * sizeof(float)));
  return static_cast<bool>(out);
}

bool SaveDisparityPng16(const std::string& path, const DisparityResult& result, float scale) {
  cv::Mat png(result.height, result.width, CV_16UC1);
  for (int y = 0; y < result.height; ++y) {
    auto* row = png.ptr<uint16_t>(y);
    for (int x = 0; x < result.width; ++x) {
      float v = result.disparity[static_cast<size_t>(y) * result.width + x];
      if (!std::isfinite(v)) v = 0.0f;
      v = std::max(0.0f, std::min(v, 255.0f));
      row[x] = static_cast<uint16_t>(std::round(v * scale));
    }
  }
  return cv::imwrite(path, png);
}

bool SaveDisparityVis(const std::string& path, const DisparityResult& result, float vis_max) {
  if (vis_max <= 0.0f) vis_max = 192.0f;
  cv::Mat vis(result.height, result.width, CV_8UC1);
  for (int y = 0; y < result.height; ++y) {
    auto* row = vis.ptr<uint8_t>(y);
    for (int x = 0; x < result.width; ++x) {
      float v = result.disparity[static_cast<size_t>(y) * result.width + x];
      if (!std::isfinite(v)) v = 0.0f;
      v = std::max(0.0f, std::min(v, vis_max));
      row[x] = static_cast<uint8_t>(std::round(v / vis_max * 255.0f));
    }
  }
  return cv::imwrite(path, vis);
}

bool SaveDisparityMeta(const std::string& path,
                       const DisparityResult& result,
                       const DisparityStats& stats,
                       const std::string& model_path,
                       const std::string& left_path,
                       const std::string& right_path) {
  std::ofstream out(path);
  if (!out) return false;
  out << "model: " << model_path << "\n";
  out << "left: " << left_path << "\n";
  out << "right: " << right_path << "\n";
  out << "width: " << result.width << "\n";
  out << "height: " << result.height << "\n";
  out << "inference_ms: " << result.inference_ms << "\n";
  out << "disp_min: " << stats.min << "\n";
  out << "disp_max: " << stats.max << "\n";
  out << "disp_mean: " << stats.mean << "\n";
  return static_cast<bool>(out);
}

bool SaveDisparityOutputs(const std::string& output_prefix,
                          const DisparityResult& result,
                          const std::string& model_path,
                          const std::string& left_path,
                          const std::string& right_path) {
  DisparityStats stats = ComputeStats(result);
  bool ok = true;
  ok = SaveDisparityBin(output_prefix + ".bin", result) && ok;
  ok = SaveDisparityPng16(output_prefix + "_disp_x256.png", result) && ok;
  ok = SaveDisparityVis(output_prefix + "_vis.png", result) && ok;
  ok = SaveDisparityMeta(output_prefix + "_meta.txt", result, stats, model_path, left_path, right_path) && ok;
  std::printf("disp shape: %dx%d, min=%.4f max=%.4f mean=%.4f\n",
              result.width, result.height, stats.min, stats.max, stats.mean);
  std::printf("saved: %s.bin, %s_disp_x256.png, %s_vis.png, %s_meta.txt\n",
              output_prefix.c_str(), output_prefix.c_str(), output_prefix.c_str(), output_prefix.c_str());
  return ok;
}

}  // namespace banet2d
