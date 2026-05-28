# BANet2D RK3588 C++ Deployment

This directory is a self-contained RK3588 deployment project for the BANet2D stereo model. It is intended to be copied to the RK3588 board or packaged as a tarball with the model, runtime libraries, sample images, scripts, and C++ source together.

## Model

- Model: BANet2D
- Max disparity: 192
- RKNN file: `models/checkpoint_epoch_195_int8.rknn`
- Inputs:
  - `left_img`: RGB uint8, NHWC, 480x640
  - `right_img`: RGB uint8, NHWC, 480x640
- Output:
  - float disparity map, 480x640

The C++ preprocessing uses OpenCV `cv::imread`, optional `cv::resize`, and explicit BGR to RGB conversion before passing NHWC uint8 buffers to RKNN Runtime.

## Directory layout

```text
rk3588_cpp/
├── CMakeLists.txt
├── README.md
├── include/banet2d/        # public C++ headers
├── src/                    # inference, image utils, RKNN utils, postprocess
├── app/main.cpp            # CLI demo
├── models/                 # RKNN model
├── samples/                # stereo sample images
├── third_party/            # vendored RKNN Runtime and OpenCV aarch64 runtime
├── scripts/                # prepare/build/run/check/package scripts
├── output/                 # inference outputs
└── package/                # generated tarballs
```

## Prepare dependencies on host

Run this on the host before copying to the board:

```bash
cd /data2/shendu/app/banet2d/rk3588_cpp
./scripts/prepare_deps.sh /data2/shendu/code/ruoyu/rknpu2
```

This copies these files into the deployment project, not symlinks:

- RKNN header: `third_party/rknn/include/rknn_api.h`
- RKNN runtime: `third_party/rknn/lib/aarch64/librknnrt.so`
- OpenCV aarch64 runtime: `third_party/opencv/opencv-linux-aarch64/`
- Model: `models/checkpoint_epoch_195_int8.rknn`
- Samples: `samples/left.png`, `samples/right.png`

## Copy to RK3588 board

Source-tree deployment:

```bash
scp -r /data2/shendu/app/banet2d/rk3588_cpp linaro@RK3588_IP:/home/linaro/banet2d_rk3588_src
```

Replace `linaro@RK3588_IP` with the actual board login target.

## Build on board

```bash
ssh linaro@RK3588_IP
cd /home/linaro/banet2d_rk3588_src
./scripts/build_on_board.sh
```

The install tree is generated at:

```text
install/banet2d_rk3588/
├── bin/banet2d_demo
├── lib/librknnrt.so
├── lib/libopencv_*        # copied when OpenCV is provided as runtime/shared files; static builds may not need .so files
├── models/checkpoint_epoch_195_int8.rknn
├── samples/left.png
├── samples/right.png
├── scripts/run_demo.sh
├── scripts/check_runtime.sh
├── output/
└── README.md
```

## Check runtime on board

```bash
cd /home/linaro/banet2d_rk3588_src/install/banet2d_rk3588
./scripts/check_runtime.sh
```

The script checks:

- current architecture
- `/dev/rknpu*`
- demo executable
- RKNN model
- `librknnrt.so`
- OpenCV libraries
- sample images
- `ldd ./bin/banet2d_demo` missing libraries

## Run demo on board

```bash
cd /home/linaro/banet2d_rk3588_src/install/banet2d_rk3588
./scripts/run_demo.sh
```

Equivalent manual command:

```bash
export LD_LIBRARY_PATH=$PWD/lib:$LD_LIBRARY_PATH
./bin/banet2d_demo \
  --model ./models/checkpoint_epoch_195_int8.rknn \
  --left ./samples/left.png \
  --right ./samples/right.png \
  --output ./output/demo \
  --warmup 1 \
  --repeat 1
```

Expected outputs:

```text
output/demo.bin
output/demo_disp_x256.png
output/demo_vis.png
output/demo_meta.txt
```

`demo.bin` is raw float32 disparity in row-major 480x640 layout. Expected size is:

```text
480 * 640 * 4 = 1228800 bytes
```

`demo_disp_x256.png` is a 16-bit PNG where each pixel is `round(disparity * 256)`. `demo_vis.png` is an 8-bit visualization clipped to disparity 192.

## Run custom stereo images

Images should already be 640x480. If not, pass `--resize` explicitly.

```bash
export LD_LIBRARY_PATH=$PWD/lib:$LD_LIBRARY_PATH
./bin/banet2d_demo \
  --model ./models/checkpoint_epoch_195_int8.rknn \
  --left /path/to/left.png \
  --right /path/to/right.png \
  --output ./output/custom \
  --warmup 3 \
  --repeat 20
```

Use `--resize` only when you intentionally accept resizing input images to 640x480:

```bash
./bin/banet2d_demo --left /path/to/left.png --right /path/to/right.png --resize
```

## Package install tree

After building on the board:

```bash
cd /home/linaro/banet2d_rk3588_src
./scripts/package.sh
```

This creates:

```text
package/banet2d_rk3588_deploy.tar.gz
```

You can copy and extract that package on another RK3588 board with a compatible RKNN driver.

## CMake modes

Default vendored mode uses `third_party/`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBANET2D_VENDOR_DEPS=ON
```

External mode uses an existing RKNPU2 checkout:

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBANET2D_VENDOR_DEPS=OFF \
  -DRKNPU2_ROOT=/path/to/rknpu2
```

## Troubleshooting

### `librknnrt.so` not found

Run from the install directory via `./scripts/run_demo.sh`, or set:

```bash
export LD_LIBRARY_PATH=$PWD/lib:$LD_LIBRARY_PATH
```

### OpenCV libraries not found

Confirm OpenCV libraries were installed:

```bash
ls lib/libopencv_*
ldd ./bin/banet2d_demo | grep 'not found'
```

If missing, rebuild and reinstall from the source tree:

```bash
./scripts/build_on_board.sh
```

### `/dev/rknpu*` missing

The board kernel driver is not exposing the NPU device. Check the RK3588 image, RKNN driver installation, and kernel logs.

### NPU permission denied

Run as a user with permission to access `/dev/rknpu*`, or adjust the board udev/group permissions.

### Wrong image size

The default path rejects non-640x480 inputs. Use images with the exported model size, or pass `--resize` when resizing is acceptable.

### BGR/RGB mismatch

OpenCV loads BGR by default. This project explicitly converts BGR to RGB in `PrepareRgbImage`, matching the exported ONNX/RKNN input convention used during conversion.

### RKNN runtime/driver mismatch

Use RKNN Runtime from the same RKNPU2 release family as the board driver and conversion toolkit. This deployment was prepared for RKNN Toolkit / RKNPU2 1.5.2.
