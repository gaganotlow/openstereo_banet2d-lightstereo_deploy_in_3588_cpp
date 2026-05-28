from rknn.api import RKNN

ONNX_MODEL = 'lightstereo_m_orin_480x640.onnx'
RKNN_MODEL = 'lightstereo_m_orin_480x640.rknn'

rknn = RKNN(verbose=True)

ret = rknn.config(
    mean_values=[[123.675, 116.28, 103.53], [123.675, 116.28, 103.53]],
    std_values=[[58.395, 57.12, 57.375], [58.395, 57.12, 57.375]],
    target_platform='rk3588',
    optimization_level=2
)
if ret != 0:
    raise RuntimeError('RKNN config failed')

ret = rknn.load_onnx(model=ONNX_MODEL)
if ret != 0:
    raise RuntimeError('RKNN load_onnx failed')

ret = rknn.build(do_quantization=False)
if ret != 0:
    raise RuntimeError('RKNN build failed')

ret = rknn.export_rknn(RKNN_MODEL)
if ret != 0:
    raise RuntimeError('RKNN export failed')

print(f'✅ {RKNN_MODEL}')
rknn.release()
