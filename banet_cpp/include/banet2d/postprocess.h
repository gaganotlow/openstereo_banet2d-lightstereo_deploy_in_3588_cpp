#pragma once

#include "banet2d/types.h"

#include <string>

namespace banet2d {

DisparityStats ComputeStats(const DisparityResult& result);
bool SaveDisparityBin(const std::string& path, const DisparityResult& result);
bool SaveDisparityPng16(const std::string& path, const DisparityResult& result, float scale = 256.0f);
bool SaveDisparityVis(const std::string& path, const DisparityResult& result, float vis_max = 192.0f);
bool SaveDisparityMeta(const std::string& path,
                       const DisparityResult& result,
                       const DisparityStats& stats,
                       const std::string& model_path,
                       const std::string& left_path,
                       const std::string& right_path);
bool SaveDisparityOutputs(const std::string& output_prefix,
                          const DisparityResult& result,
                          const std::string& model_path,
                          const std::string& left_path,
                          const std::string& right_path);

}  // namespace banet2d
