#!/bin/bash

data_dir="datas/calibration/test02"
grid_number="8x6"

./build/examples/calibration/calibration.bin \
    --mode 3 --omit_distortion \
    --grid_square_size_mm 40.0 \
    --grid_number_inner_corners ${grid_number} \
    --camera_parameter_folder ${data_dir}/ \
    --calibration_image_dir ${data_dir}/extrinsics \
    --number_cameras 2

