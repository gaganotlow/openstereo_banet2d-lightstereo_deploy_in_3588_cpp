# openstereo_banet2d-lightstereo_deploy_in_3588_cpp

RK3588 上的 OpenStereo / BANet2D / LightStereo C++ 部署工程整理。

## 目录说明

- `banet_cpp/`：BANet2D C++ 推理部署，当前可正常使用。
- `lightstereo_cpp/`：LightStereo C++ 推理部署，当前可正常使用。
- `lightstereo/`：LightStereo 相关工程，当前推理存在问题，待修复。

## 当前状态

已完成三个文件夹的整理与提交，其中 `banet_cpp` 和 `lightstereo_cpp` 可作为 RK3588 部署参考；`lightstereo` 暂不建议直接用于推理。

## 参考

- 导出 ONNX 的操作参考：[OpenStereo issue #212](https://github.com/XiandaGuo/OpenStereo/issues/212)
