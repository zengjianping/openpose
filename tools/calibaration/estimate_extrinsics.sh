#!/bin/bash

data_dir="datas/calibration/test03"
grid_number="8x6"

./build/examples/calibration/calibration.bin \
    --mode 2 --omit_distortion \
    --grid_square_size_mm 28.0 \
    --grid_number_inner_corners ${grid_number} \
    --camera_parameter_folder ${data_dir}/ \
    --calibration_image_dir ${data_dir}/extrinsics \
    --cam0 0 --cam1 1




